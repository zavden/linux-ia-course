#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "threads_joined=4"
echo "$OUT" | grep -q "total=500500"
echo "$OUT" | grep -q "expected=500500"

echo "E01 OK"
