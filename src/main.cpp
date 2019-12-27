#include <cstdint>
#include <iostream>
#include <vector>

//#include <lyra/lyra.hpp>
#include <terra/terra.hpp>

int32_t main(int32_t argc, char** argv)
{
    auto width = 1000;
    auto height = 500;
    auto radius = 10.0;
    auto samples = 100;

    auto sampler = terra::poisson_disc_sampler(width, height, radius, samples);

    std::count << sampler.sample();

    system("pause");

    return 0;
}