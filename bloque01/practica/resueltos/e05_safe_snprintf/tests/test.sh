#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "truncated=yes"
echo "$OUT" | grep -q "out='id=42 use'"

echo "E05 OK"
