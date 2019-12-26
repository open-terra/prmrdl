#include <cstdint>
#include <iostream>
#include <vector>

//#include <lyra/lyra.hpp>
#include <terra/terra.hpp>

int32_t main(int32_t argc, char** argv)
{
    constexpr auto width = 1000;
    constexpr auto height = 500;
    constexpr auto radius = 10.0;
    constexpr auto samples = 100;

    auto sampler = terra::poisson_disc_sampler(width, height, radius, samples);

    std::count << sampler.sample();

    system("pause");

    return 0;
}