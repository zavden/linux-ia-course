#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "limited=1"
echo "$OUT" | grep -q "quota=50000"
echo "$OUT" | grep -q "period=100000"
echo "$OUT" | grep -q "weight=100"

echo "E04 OK"
