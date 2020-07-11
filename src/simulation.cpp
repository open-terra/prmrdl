#include "simulation.hpp"

#include <array>
#include <tuple>
#include <string_view>

#include "usage.hpp"

bool configure_simulation(const argh::parser& cmdl, const output& out)
{
    const std::array<std::tuple<std::string_view, std::string_view>, 2> sim_types =
    {
        std::make_tuple("hydraulic", "desc here pls"),
        std::make_tuple("thermal", "another desc here pls")
    };

    auto type = cmdl[2];
    if (type == "list")
    {
        print_title();
        std::cout << "Simulation types:" << std::endl;
        for (const auto& [sim, desc] : sim_types)
        {
            std::cout << sim << " - " << desc << std::endl;
        }
    }

    bool found = false;
    for (const auto& [sim, _] : sim_types)
    {
        if (type == sim)
        {
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