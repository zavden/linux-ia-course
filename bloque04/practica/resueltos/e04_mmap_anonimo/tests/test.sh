#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "first=0"
echo "$OUT" | grep -q "last=255"

echo "E04 OK"
