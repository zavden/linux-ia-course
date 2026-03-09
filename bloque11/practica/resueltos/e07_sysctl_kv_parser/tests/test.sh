#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "total=6"
echo "$OUT" | grep -q "net=2"
echo "$OUT" | grep -q "vm=1"
echo "$OUT" | grep -q "kernel=2"
echo "$OUT" | grep -q "unsafe=0"

echo "E07 OK"
