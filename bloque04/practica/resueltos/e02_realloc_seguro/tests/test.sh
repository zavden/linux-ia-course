#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "cap=32"
echo "$OUT" | grep -q "prefix=ABCDEF"

echo "E02 OK"
