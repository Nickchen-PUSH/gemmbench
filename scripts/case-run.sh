#!/bin/bash
set -e
project_root=$(dirname "$(dirname "$0")")
cd "$project_root" || exit 1

tag="mac-$(date +%Y%m%d-%H%M%S)"
mkdir -p "results/$tag"

sizes=(32)
ops=("NaiveGemmOp")

for size in "${sizes[@]}"; do
    for op in "${ops[@]}"; do
        ./gemmbench run --op $op --sample "cases/case_${size}x${size}x${size}.bin" --output "results/$tag/${op}_${size}x${size}x${size}.json"
    done
done