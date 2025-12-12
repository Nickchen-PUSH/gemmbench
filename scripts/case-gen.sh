#!/bin/bash
set -e
project_root=$(dirname "$(dirname "$0")")
cd "$project_root" || exit 1

sizes=(96 224 480)

for size in "${sizes[@]}"; do
    ./bin/gemmbench generate --m $size --n $size --k $size --sample "cases/case_${size}x${size}x${size}.bin"
done