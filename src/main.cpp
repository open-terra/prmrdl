#include <cstdint>
#include <iostream>
#include <optional>
#include <vector>

#include <terra/terra.hpp>

tfloat terrain_epsilon = 0.0001;

inline bool end_function(const terra::dynarray<tfloat>& heights,
                         terra::dynarray<tfloat>& heights_old)
{
    for (size_t i = 0; i < heights.size(); ++i)
    {
        if (terra::math::abs(heights[i] - heights_old[i]) > terrain_epsilon)
        {
            return true;
        }
    }

    return false;
}

int32_t main(int32_t argc, char** argv)
{
#ifdef TERRA_USE_OPENCL
    terra::compute::engine_cl engine;
    engine.add_source("kernels/uplift.cl");
    engine.init();
#endif

    tfloat uplift_per_year = 5.01e-4;
    tfloat erosion_rate = 5.61e-7;
    tfloat time_scale = 2.5e5;

    tfloat k = 1.0;
    tfloat m = 0.5;
    tfloat n = 1.0;

    tfloat uplift_factor = uplift_per_year * time_scale;
    k = erosion_rate * time_scale;

    size_t width = 50000;
    size_t height = 50000;
    tfloat radius = 100.0;
    size_t samples = 100;

    std::vector<terra::vec2> points;
    std::unique_ptr<terra::hash_grid> hash_grid;
    {
        auto sampler = terra::poisson_disc_sampler();
        points = sampler.sample(width, height, radius, samples, hash_grid.get());
    }
    const size_t node_count = points.size();

    std::cout << "Points sampled: " << points.size() << std::endl;

    // verify points are within the sample region
    {
        size_t valid_points = 0;
        terra::rect<tfloat> bounds(0.0, 0.0, width, height);
        for (const auto& p : points)
        {
            if (bounds.within_extent(p))
            {
                ++valid_points;
            }
        }

        std::cout << "Valid Points sampled: " << valid_points << std::endl;
    }

    terra::dynarray<terra::triangle> tris(0);
    {
        terra::delaunay d;
        d.triangulate(points);
        tris = terra::dynarray<terra::triangle>(d.triangles.size() / 3);

        std::cout << "Triangles created: " << d.triangles.size() << std::endl;

        for (size_t i = 0; i < tris.size(); ++i)
        {
            size_t index = i * 3;
            const size_t p0 = index;
            const size_t p1 = index + 1;
            const size_t p2 = index + 2;

            const size_t v0 = d.triangles[p0];
            const size_t v1 = d.triangles[p1];
            const size_t v2 = d.triangles[p2];

            tris[i] = terra::triangle(v0, v1, v2);
        }

        std::cout << "Triangles created: " << tris.size() << std::endl;
    }

    terra::undirected_graph graph(node_count, tris);
    std::cout << "Graph edges: " << graph.num_edges() << std::endl;

    terra::dynarray<tfloat> areas(node_count);
    {
        terra::dynarray<terra::polygon> cells(points.size());
        {
            terra::voronoi v;
            v.generate(points, {0.0f, 0.0f, static_cast<tfloat>(width), static_cast<tfloat>(height)}, cells);
        }

        for (size_t i = 0; i < node_count; ++i)
        {
            const auto& cell = cells[i];
            const auto& centre = points[i];
            if (cell.vertices.size() > 0)
            {
                areas[i] = cell.area(centre);
            }
            else
            {
                std::cout << "no vertices at: #" << i << " - { " << centre.x << "," << centre.y << " }" << std::endl;
            }
        }
    }

    std::cout << "Voronoi partition completed, areas computed" << std::endl;

    terra::dynarray<tfloat> heights(node_count);
    {
        terra::dynarray<tfloat> heights_old(node_count);
        std::fill(heights.begin(), heights.end(), 0.0f);

        terra::uplift uplift;
        {
            terra::linear_uplift uplift_func(width, height, 0.01, 1.0);
            uplift = terra::uplift(uplift_func, points, heights, uplift_factor);
        }
        terra::flow_graph flow_graph(node_count, graph, areas, heights);

        terra::stream_power_equation fluvial_erosion(k, time_scale, points, flow_graph, areas, uplift.uplifts, heights);

        size_t itterations = 0;
        do
        {
            std::copy(heights.begin(), heights.end(), heights_old.begin());

            // update
            flow_graph.update();

            // erode
            fluvial_erosion.update();
        }
        while (end_function(heights, heights_old) && (++itterations) < 300);

        std::cout << "Graph converged in " << itterations << "iterations" << std::endl;
    }

    terra::dynarray<terra::vec3> verts(points.size());
    for (size_t i = 0; i < node_count; ++i)
    {
        const auto& p = points[i];
        verts[i] = { p.x, p.y, heights[i] };
    }

    terra::io::obj::write_obj("terrain.obj", verts, tris);

    return 0;
}
