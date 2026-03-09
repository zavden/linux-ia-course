#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "load_A="
echo "$OUT" | grep -q "load_B="
TOT=$(echo "$OUT" | sed -n 's/.*total=\([0-9][0-9]*\).*/\1/p')
[ -n "$TOT" ] && [ "$TOT" -gt 0 ]

echo "E10 OK"
