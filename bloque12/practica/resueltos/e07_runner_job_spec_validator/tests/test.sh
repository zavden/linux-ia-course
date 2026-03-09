#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "valid=4"
echo "$OUT" | grep -q "invalid=1"
echo "$OUT" | grep -q "heavy=2"

echo "E07 OK"
