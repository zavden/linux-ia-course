#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "modules=3"
echo "$OUT" | grep -q "in_use=2"
echo "$OUT" | grep -q "dep_edges=1"

echo "E01 OK"
