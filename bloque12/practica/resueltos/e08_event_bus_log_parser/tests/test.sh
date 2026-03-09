#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "total=6"
echo "$OUT" | grep -q "publish=2"
echo "$OUT" | grep -q "consume=3"
echo "$OUT" | grep -q "retry=1"
echo "$OUT" | grep -q "errors=2"
echo "$OUT" | grep -q "max_lat=240"

echo "E08 OK"
