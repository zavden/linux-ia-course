#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "level=2"
echo "$OUT" | grep -q "err=1"
echo "$OUT" | grep -q "info=2"
echo "$OUT" | grep -q "dbg=0"

echo "E02 OK"
