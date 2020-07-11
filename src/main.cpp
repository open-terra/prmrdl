#include <array>
#include <cstdint>
#include <iostream>
#include <functional>
#include <tuple>
#include <string_view>
#include <vector>

#include "argh.h"

#include "usage.hpp"
#include "output.hpp"
#include "lstgtufe.hpp"
#include "noise.hpp"
#include "simulation.hpp"

struct function
{
    typedef std::function<bool(const argh::parser&, const output&)> callback_t;

    function(const std::string_view& name, const std::string_view& usage, callback_t callback) : name(name), usage(usage), callback(callback)
    {
    }

    std::string_view name;
    std::string_view usage;
    callback_t callback;
};

std::array<function, 3> functions =
{
    function("lstgtufe",   "usage", configure_lstgtufe),
    function("noise",      "usage", configure_noise),
    function("simulation", "usage", configure_simulation)
};

int32_t main(int32_t argc, char** argv)
{
    auto cmdmode = argh::parser::PREFER_PARAM_FOR_UNREG_OPTION
                 | argh::parser::SINGLE_DASH_IS_MULTIFLAG;
    auto cmdl = argh::parser(argc, argv, cmdmode);

    if (cmdl({"-h", "--help"}))
    {
        print_title();
        print_usage();

        return 0;
    }

    auto function = cmdl[1];

    std::string out_path;
    cmdl({"-o", "--output"}, "temp_hf.png") >> out_path;
    output out = { out_path, output_type::heightfield };

    bool found = false;
    for (const auto& f : functions)
    {
        if (function == f.name)
        {
            if (!f.callback(cmdl, out))
            {
                print_title();
                std::cout << f.usage << std::endl;
            }

            found = true;
            break;
        }
    }

    if (!found)
    {
        std::cout << "Invalid function passed" << std::endl;
        print_usage();
    }

    return 0;
}
