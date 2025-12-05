#!/bin/bash
set -e
project_root=$(dirname "$(dirname "$0")")
cd "$project_root" || exit 1

cmake -B build \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS_RELEASE="-O3 -march=native"

cmake --build build -j8