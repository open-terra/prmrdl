#pragma once

#include "argh.h"
#include "output.hpp"

bool configure_simulation(const argh::parser& cmdl, const output& out);

void hydraulic_erosion(const output& out);
void thermal_erosion(const output& out);
