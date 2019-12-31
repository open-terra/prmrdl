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
        terra::delaunator d;
        d.triangulate(points);
        std::cout << "Triangles created: " << d.triangles.size() << std::endl;

        graph = terra::undirected_graph(points.size(), d.triangles.size());

        for (size_t i = 0; i < d.triangles.size(); i+=3)
        {
            const size_t p0 = i;
            const size_t p1 = i + 1;
            const size_t p2 = i + 2;

            const size_t v0 = d.triangles[p0];
            const size_t v1 = d.triangles[p1];
            const size_t v2 = d.triangles[p2];

            graph.add_edge(v0, v1);
            graph.add_edge(v1, v2);
            graph.add_edge(v2, v0);
        }
    }

    std::cout << "Graph edges: " << graph.num_edges() << std::endl;

    system("pause");

    return 0;
}