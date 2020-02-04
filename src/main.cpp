#include <cstdint>
#include <iostream>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include <terra/terra.hpp>

tfloat k = 1.0;
tfloat m = 0.5;
tfloat n = 1.0;
tfloat terrain_epsilon = 0.001;

inline void slope_update(const terra::dynarray<size_t>& flow,
                         const std::vector<terra::vec2>& points,
                         const terra::dynarray<tfloat>& heights,
                         terra::dynarray<tfloat>& slope)
{
    for (size_t i = 0; i < heights.size(); ++i)
    {
        if (flow[i] != terra::flow_graph::node_lake)
        {
            const auto dh = heights[flow[i]] - heights[i];
            const auto dist = glm::distance(points[flow[i]], points[i]);
            slope[i] = dh / dist;
        }
        else
        {
            slope[i] = 0.0;
        }
    }
}

inline tfloat stream_power_equation(const tfloat height,
                                    const tfloat area,
                                    const tfloat slope)
{
    return height - (k * std::pow(area, m) * std::pow(slope, n));
}

inline bool end_function(const terra::dynarray<tfloat>& heights,
                         terra::dynarray<tfloat>& heights_old)
{
    for (size_t i = 0; i < heights.size(); ++i)
    {
        heights_old[i] = heights[i] - heights_old[i];
    }

    tfloat sum = terra::math::sum<tfloat>(heights_old);

    return (sum / heights.size()) > terrain_epsilon;
}

int32_t main(int32_t argc, char** argv)
{
#ifdef TERRA_USE_OPENCL
    terra::compute::engine_cl engine;
    engine.add_source("kernels/uplift.cl");
    engine.init();
#endif

    tfloat width = 1000.0;
    tfloat height = 500.0;
    tfloat radius = 10.0;
    tfloat samples = 100;

    std::vector<terra::vec2> points;
    {
        auto sampler = terra::poisson_disc_sampler();
        points = sampler.sample(width, height, radius, samples);
    }

    std::cout << "Points sampled: " << points.size() << std::endl;

    terra::dynarray<terra::triangle> tris;
    {
        terra::delaunator d;
        d.triangulate(points);
        tris = terra::dynarray<terra::triangle>(d.triangles.size() / 3);
        for (size_t i = 0; i < d.triangles.size(); i += 3)
        {
            const size_t p0 = i;
            const size_t p1 = i + 1;
            const size_t p2 = i + 2;

            const size_t v0 = d.triangles[p0];
            const size_t v1 = d.triangles[p1];
            const size_t v2 = d.triangles[p2];

            tris[i] = {v0, v1, v2};
        }

        std::cout << "Triangles created: " << tris.size() << std::endl;
    }

    terra::undirected_graph graph(points.size(), tris);
    std::cout << "Graph edges: " << graph.num_edges() << std::endl;

    terra::dynarray<tfloat> areas(points.size());
    {
        terra::dynarray<terra::polygon> cells(points.size());
        {
            terra::voronoi v;
            v.generate(points, {0.0, width, 0.0, height}, cells);
        }

        for (size_t i = 0; i < cells.size(); ++i)
        {
            const auto& cell = cells[i];
            const auto& centre = points[cell.centre_idx];
            areas[i] = cell.area(centre);
        }
    }

    std::cout << "Voronoi partition completed, areas computed" << std::endl;

    terra::dynarray<tfloat> heights(points.size());
    {
        terra::dynarray<tfloat> heights_old(points.size());
        for (auto& h : heights)
        {
            h = 0.0;
        }

        terra::uplift uplift(terra::bitmap(), points, heights);
        terra::flow_graph flow_graph(points.size(), graph, areas, heights);
        terra::dynarray<tfloat> slopes(points.size());

        do
        {
            std::copy(heights.begin(), heights.end(), heights_old.begin());

            // update
#ifdef TERRA_USE_OPENCL
            uplift.update(engine);
#else
            uplift.update();
#endif
            flow_graph.update();
            slope_update(flow_graph.flow, points, heights, slopes);

            // erode
            for (size_t i = 0; i < points.size(); ++i)
            {
                heights[i] = stream_power_equation(heights[i], areas[i], slopes[i]);
            }
        }
        while (end_function(heights, heights_old));
    }

    terra::dynarray<terra::vec3> verts(points.size());
    for (size_t i = 0; i < points.size(); ++i)
    {
        const auto& p = points[i];
        verts[i] = { p.x, p.y, heights[i] };
    }

    terra::io::obj::write_obj("terrain.obj", verts, tris);

    return 0;
}
