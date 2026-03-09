#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "grow_to=4"
echo "$OUT" | grep -q "grow_to=8"
echo "$OUT" | grep -q "len=20"
echo "$OUT" | grep -q "last=20"

echo "E01 OK"
