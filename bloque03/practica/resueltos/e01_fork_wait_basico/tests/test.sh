#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "code=10"
echo "$OUT" | grep -q "code=11"
echo "$OUT" | grep -q "code=12"

echo "E01 OK"
