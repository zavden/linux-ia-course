#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "kind="
echo "$OUT" | grep -q "after_soft="
echo "$OUT" | grep -q "allocated_chunks="

echo "E07 OK"
