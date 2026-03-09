#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "modules=3"
echo "$OUT" | grep -q "busy=2"
echo "$OUT" | grep -q "top=xfs"

echo "E02 OK"
