#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "valid=4"
echo "$OUT" | grep -q "invalid=1"
echo "$OUT" | grep -q "global=2"
echo "$OUT" | grep -q "rotatable=3"

echo "E06 OK"
