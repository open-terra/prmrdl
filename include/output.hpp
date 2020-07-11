#pragma once

#include <string>

enum struct output_type
{
    heightfield,
    model
};

struct output
{
    const std::string& path;
    const output_type type;
};
