#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "final_value=100"
echo "$OUT" | grep -q "version=10"
echo "$OUT" | grep -q "write_ops=10"

echo "E07 OK"
