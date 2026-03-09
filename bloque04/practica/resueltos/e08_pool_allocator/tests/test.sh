#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "alloc_ok="
echo "$OUT" | grep -q "capacity=1048576"

echo "E08 OK"
