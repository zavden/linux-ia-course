#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "produced=20"
echo "$OUT" | grep -q "consumed=20"
echo "$OUT" | grep -q "remaining=0"
echo "$OUT" | grep -q "done=1"

echo "E05 OK"
