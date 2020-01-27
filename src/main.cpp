#include <cstdint>
#include <iostream>
#include <vector>

//#include <lyra/lyra.hpp>
#include <terra/terra.hpp>

int32_t main(int32_t argc, char** argv)
{
    auto width = 1000.0;
    auto height = 500.0;
    auto radius = 10.0;
    auto samples = 100;

    std::vector<terra::vec2> points;
    {
        auto sampler = terra::poisson_disc_sampler();
        points = sampler.sample(width, height, radius, samples);
    }

    std::cout << "Points sampled: " << points.size() << std::endl;

    terra::undirected_graph graph;
    {
        std::vector<terra::triangle> tris;
        {
            terra::delaunator d;
            d.triangulate(points);
            tris.reserve(d.triangles.size() / 3);
            for (size_t i = 0; i < triangles.size(); i += 3)
            {
                const size_t p0 = i;
                const size_t p1 = i + 1;
                const size_t p2 = i + 2;

                const size_t v0 = triangles[p0];
                const size_t v1 = triangles[p1];
                const size_t v2 = triangles[p2];

                tris.push_back({v0, v1, v2});
            }
        }

        std::cout << "Triangles created: " << tris.size() << std::endl;

        terra::io::obj::write_obj("graph.obj", points, tris);
        graph = terra::undirected_graph(points.size(), tris);
    }

    std::cout << "Graph edges: " << graph.num_edges() << std::endl;

    return 0;
}