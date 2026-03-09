#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "r0=7"
echo "$OUT" | grep -q "r1=7"
echo "$OUT" | grep -q "r2=10"
echo "$OUT" | grep -q "r3=ERR"

echo "E08 OK"
