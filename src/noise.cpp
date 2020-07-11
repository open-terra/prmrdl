#include "noise.hpp"

#include <array>
#include <functional>
#include <string_view>

#include <terra/terra.hpp>

#include "usage.hpp"

terra::dynarray<tfloat> fbm_noise(size_t, size_t, size_t, size_t, float, size_t, size_t,  float, float);
terra::dynarray<tfloat> billowy_noise(size_t, size_t, size_t, size_t, float, size_t, size_t,  float, float);
terra::dynarray<tfloat> ridged_noise(size_t, size_t, size_t, size_t, float, size_t, size_t,  float, float);
terra::dynarray<tfloat> erosive_noise(size_t, size_t, size_t, size_t, float, size_t, size_t,  float, float);

struct noise_function
{
    typedef std::function<terra::dynarray<tfloat>(size_t, size_t, size_t, size_t, float, size_t, size_t, float, float)> callback_t;

    noise_function(const std::string_view& name, const std::string_view& usage, callback_t callback) : name(name), usage(usage), callback(callback)
    {
    }

    std::string_view name;
    std::string_view usage;
    callback_t callback;
};

bool configure_noise(const argh::parser& cmdl, const output& out)
{
    const std::array<noise_function, 4> noise_types =
    {
        noise_function("fBm", "another desc here pls", fbm_noise),
        noise_function("billowy", "another desc here pls", billowy_noise),
        noise_function("ridged", "another desc here pls", ridged_noise),
        noise_function("erosive", "desc here pls", erosive_noise)
    };

    auto type = cmdl[2];
    if (type == "list")
    {
        print_title();
        std::cout << "Noise types:" << std::endl;
        for (const auto& n : noise_types)
        {
            std::cout << n.name << " - " << n.usage << std::endl;
        }
    }

    size_t x_off;
    size_t y_off;
    size_t x_size;
    size_t y_size;
    float scale;
    size_t seed;
    size_t octaves;
    float persistence;
    float lacunarity;

    cmdl(3, 0ull) >> x_off;
    cmdl(4, 0ull) >> y_off;
    cmdl(5, 512ull) >> x_size;
    cmdl(6, 512ull) >> y_size;
    cmdl(7, 0.125f) >> scale;
    cmdl(8, 2552ull) >> seed;
    cmdl(9, 6ull) >> octaves;
    cmdl(10, 0.5f) >> persistence;
    cmdl(11, 2.0f) >> lacunarity;

    bool found = false;
    for (const auto& n : noise_types)
    {
        if (type == n.name)
        {
            auto noise_set = n.callback(x_off, y_off, x_size, y_size, scale, seed, octaves, persistence, lacunarity);
            if (noise_set.size() > 0)
            {
                terra::heightfield h;
                auto bitmap = h.raster<float>(512, 512, 0.0f, 1.0f, noise_set);

                terra::io::write_image(out.path, bitmap);
            }

            found = true;
            break;
        }
    }

    if (!found)
    {
        // help stuff here
        return false;
    }

    return true;
}

terra::dynarray<tfloat> fbm_noise
(
    size_t x_offset,
    size_t y_offset,
    size_t x_size,
    size_t y_size,
    float scale,
    size_t seed,
    size_t octaves,
    float persistence,
    float lacunarity)
{
    auto noise = terra::noise::fbm_noise(seed);
    noise.set_octaves(octaves);

    return noise.noise(x_offset, y_offset, 2552, x_size, y_size, 1, scale);
}

terra::dynarray<tfloat> billowy_noise
(
    size_t x_offset,
    size_t y_offset,
    size_t x_size,
    size_t y_size,
    float scale,
    size_t seed,
    size_t octaves,
    float persistence,
    float lacunarity)
{
    auto noise = terra::noise::billowy_noise(seed);
    noise.set_octaves(octaves);

    return noise.noise(x_offset, y_offset, 2552, x_size, y_size, 1, scale);
}

terra::dynarray<tfloat> ridged_noise
(
    size_t x_offset,
    size_t y_offset,
    size_t x_size,
    size_t y_size,
    float scale,
    size_t seed,
    size_t octaves,
    float persistence,
    float lacunarity)
{
    auto noise = terra::noise::ridged_noise(seed);
    noise.set_octaves(octaves);

    return noise.noise(x_offset, y_offset, 2552, x_size, y_size, 1, scale);
}

terra::dynarray<tfloat> erosive_noise
(
    size_t x_offset,
    size_t y_offset,
    size_t x_size,
    size_t y_size,
    float scale,
    size_t seed,
    size_t octaves,
    float persistence,
    float lacunarity)
{
    auto noise = terra::noise::erosive_noise(seed);

    return noise.noise(x_offset, y_offset, x_size, y_size, scale, octaves);
}
