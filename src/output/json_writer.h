#pragma once
#include <string>
#include "benchmark/benchmark.h"

std::string make_json(const BenchResult &r,
                      const std::string &op,
                      int M, int N, int K);