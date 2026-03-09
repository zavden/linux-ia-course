#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "lines=3"
echo "$OUT" | grep -q "first=SET a 1"
echo "$OUT" | grep -q "last=PING"
echo "$OUT" | grep -q "pending=0"

echo "E06 OK"
