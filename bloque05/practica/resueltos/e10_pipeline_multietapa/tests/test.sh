#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "items=15"
echo "$OUT" | grep -q "final_count=15"
echo "$OUT" | grep -q "final_sum=255"
echo "$OUT" | grep -q "expected=255"

echo "E10 OK"
