#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "ready_a=1"
echo "$OUT" | grep -q "ready_b=0"
echo "$OUT" | grep -q "msg=poll_ok"

echo "E05 OK"
