#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "ready1=0"
echo "$OUT" | grep -q "ready2=1"
echo "$OUT" | grep -q "msg=ready_two"

echo "E04 OK"
