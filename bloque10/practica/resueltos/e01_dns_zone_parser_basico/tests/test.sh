#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "total=9"
echo "$OUT" | grep -q "A=2"
echo "$OUT" | grep -q "AAAA=1"
echo "$OUT" | grep -q "MX=1"

echo "E01 OK"
