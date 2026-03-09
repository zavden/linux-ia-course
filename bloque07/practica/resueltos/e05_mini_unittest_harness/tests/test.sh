#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "passed=4"
echo "$OUT" | grep -q "failed=0"

echo "E05 OK"
