#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "workers=4"
echo "$OUT" | grep -q "tasks=20"
echo "$OUT" | grep -q "processed=20"
echo "$OUT" | grep -q "sum_sq=2870"

echo "E09 OK"
