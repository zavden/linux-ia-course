#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "expected=100000"
echo "$OUT" | grep -q "observed=50000"
echo "$OUT" | grep -q "race_detected=1"

echo "E03 OK"
