#include "lstgtufe.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

#include <terra/terra.hpp>

tfloat terrain_epsilon = 0.0001;

inline bool end_function(const terra::dynarray<tfloat>& heights,
                         const terra::dynarray<tfloat>& heights_old)
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

bool configure_lstgtufe(const argh::parser& cmdl, const output& out)
{
    size_t width            = 50000;
    size_t height           = 50000;
    float radius            = 100.0;
    size_t samples          = 100;
    float uplift_per_year   = 5.01e-4;
    float erosion_rate      = 5.61e-7;
    float time_scale        = 2.5e5;
    size_t max_itterations  = 300;

    // Poisson disc sampler options
    cmdl(2, 50000) >> width;
    cmdl(3, 50000) >> height;
    cmdl(4, 100.0) >> radius;
    cmdl(5, 100)   >> samples;

    // Simulation options
    cmdl(6,  5.01e-4) >> uplift_per_year;
    cmdl(7,  5.61e-7) >> erosion_rate;
    cmdl(8,  2.5e5)   >> time_scale;
    cmdl(9,  300)     >> max_itterations;

    lstgtufe(out,
             width,
             height,
             radius,
             samples,
             uplift_per_year,
             erosion_rate,
             time_scale);

    return true;
}

void lstgtufe(const output& out,
              size_t width,
              size_t height,
              float radius,
              size_t samples,
              float uplift_per_year,
              float erosion_rate,
              float time_scale,
              size_t max_itterations)
{
    tfloat uplift_factor = uplift_per_year * time_scale;

    tfloat k = erosion_rate * time_scale;
    tfloat m = 0.5;
    tfloat n = 1.0;

    std::vector<terra::vec2> points;
    std::unique_ptr<terra::hash_grid> hash_grid;
    {
        terra::hash_grid* temp_grid = nullptr;
        auto sampler = terra::poisson_disc_sampler();
        points = sampler.sample(width, height, radius, samples, &temp_grid);

        hash_grid = std::unique_ptr<terra::hash_grid>(temp_grid);
    }
    const size_t node_count = points.size();

    std::cout << "Points sampled: " << points.size() << std::endl;

    terra::dynarray<terra::triangle> tris(0);
    {
        terra::delaunator d;
        auto _tris = d.triangulate(points);
        tris = terra::dynarray<terra::triangle>(_tris.size() / 3);

        for (size_t i = 0; i < tris.size(); ++i)
        {
            size_t index = i * 3;
            const size_t p0 = index;
            const size_t p1 = index + 1;
            const size_t p2 = index + 2;

            const size_t v0 = _tris[p0];
            const size_t v1 = _tris[p1];
            const size_t v2 = _tris[p2];

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
            v.generate(points, terra::rect<tfloat>(0.0f, 0.0f, static_cast<tfloat>(width), static_cast<tfloat>(height)), cells);
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

        for (size_t i = 0; i < node_count; ++i)
        {
            if (areas[i] < 0.0)
            {
                const auto& centre = points[i];
                std::cout << "bad area \"" << areas[i] << "\" at: " << i << " - { " << centre.x << "," << centre.y << " }" << std::endl;
                
                static const auto area = terra::math::PI * (radius * radius);
                areas[i] = area;
            }

            // area needs to be in m^2?
            // areas[i] *= 1'000'000.0;
        }
    }

    std::cout << "Voronoi partition completed, areas computed" << std::endl;

    terra::dynarray<tfloat> heights(node_count);
    {
        terra::dynarray<tfloat> heights_old(node_count);
        std::fill(heights.begin(), heights.end(), 0.0f);

        terra::linear_uplift uplift_func(width, height, 0.01, 1.0);
        terra::uplift uplift(uplift_func, points, heights, uplift_factor);
        terra::flow_graph flow_graph(node_count, graph, areas, heights);

        terra::stream_power_equation fluvial_erosion(k, time_scale, points, flow_graph, areas, uplift.uplifts, heights);
        terra::thermal_erosion thermal_erosion(points, heights, graph, 40.0);

        size_t itterations = 0;
        do
        {
            std::copy(heights.begin(), heights.end(), heights_old.begin());

            // update
            flow_graph.update();

            // erode
            fluvial_erosion.update();
            thermal_erosion.update();
        }
        while (end_function(heights, heights_old) && (++itterations) < max_itterations);

        std::cout << "Graph converged in " << itterations << " iterations" << std::endl;
    }

    switch (out.type)
    {
        case output_type::heightfield:
        {
            terra::rasteriser r(heights, *hash_grid.get());
            auto hf = r.raster<uint8_t>(512, 512);
            auto bitmap = terra::bitmap(512, 512, 8, 1, 512 * 512, hf);

            terra::io::write_image(out.path, bitmap);
        };
        case output_type::model:
        {
            terra::dynarray<terra::vec3> verts(points.size());
            for (size_t i = 0; i < node_count; ++i)
            {
                const auto& p = points[i];
                verts[i] = { p.x, p.y, heights[i] };
            }

            terra::io::obj::write_obj(out.path, verts, tris);
        };
    }
    
}
