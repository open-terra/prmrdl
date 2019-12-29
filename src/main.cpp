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

    std::cout << "Points Sampled: " << points.size() << std::endl;

    system("pause");

    return 0;
}