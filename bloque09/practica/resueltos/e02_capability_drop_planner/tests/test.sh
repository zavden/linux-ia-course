#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "keep_n=1"
echo "$OUT" | grep -q "drop_n=3"
echo "$OUT" | grep -q "keep=0x400"

echo "E02 OK"
