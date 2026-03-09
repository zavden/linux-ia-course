#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "sum=12"
echo "$OUT" | grep -q "sub=2"

echo "E04 OK"
