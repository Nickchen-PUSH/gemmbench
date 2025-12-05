#!/bin/bash
set -e
project_root=$(dirname "$(dirname "$0")")
cd "$project_root" || exit 1

tag="mac-$(date +%Y%m%d-%H%M%S)"
mkdir -p "results/$tag"

sizes=(128 256 512)
dtypes=("float32" "bfloat16" "float16")
ops=("NaiveGemmOp")

for size in "${sizes[@]}"; do
    for dtype in "${dtypes[@]}"; do
        for op in "${ops[@]}"; do
            ./bin/gemmbench run --op $op --sample "cases/case_${size}x${size}x${size}_${dtype}.bin" --output "results/$tag/${op}_${size}x${size}x${size}_${dtype}.txt"
        done
    done
done