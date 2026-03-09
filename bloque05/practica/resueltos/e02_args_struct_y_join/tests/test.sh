#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "grand_total=9455"
echo "$OUT" | grep -q "expected=9455"

echo "E02 OK"
