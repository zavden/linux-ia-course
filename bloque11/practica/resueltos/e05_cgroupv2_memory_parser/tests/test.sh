#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "mem_limited=1"
echo "$OUT" | grep -q "mem_max=104857600"
echo "$OUT" | grep -q "mem_current=52428800"
echo "$OUT" | grep -q "swap_limited=0"

echo "E05 OK"
