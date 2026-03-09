#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "clamp=100"
echo "$OUT" | grep -q "div_ok=0"
echo "$OUT" | grep -q "div_err=-1"
echo "$OUT" | grep -q "q=4"

echo "E01 OK"
