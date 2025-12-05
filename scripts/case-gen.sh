#!/bin/bash
set -e
project_root=$(dirname "$(dirname "$0")")
cd "$project_root" || exit 1

sizes=(128 256 512)
dtypes=("float32" "bfloat16" "float16")

for size in "${sizes[@]}"; do
    for dtype in "${dtypes[@]}"; do
        ./bin/gemmbench generate --m $size --n $size --k $size --dtype $dtype --sample "cases/case_${size}x${size}x${size}_${dtype}.bin"
    done
done