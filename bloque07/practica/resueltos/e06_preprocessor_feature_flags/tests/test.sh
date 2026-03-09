#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "fast_mode=0"
echo "$OUT" | grep -q "sum=5050"

echo "E06 OK"
