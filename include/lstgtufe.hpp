#pragma once

#include "argh.h"
#include "output.hpp"

bool configure_lstgtufe(const argh::parser& cmdl, const output& out);

void lstgtufe(const output& out,
              size_t width = 50000,
              size_t height = 50000,
              float radius = 100.0,
              size_t samples = 100,
              float uplift_per_year = 5.01e-4,
              float erosion_rate = 5.61e-7,
              float time_scale = 2.5e5,
              size_t max_itterations = 300);
