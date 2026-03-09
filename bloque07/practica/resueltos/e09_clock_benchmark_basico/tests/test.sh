#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

ENS=$(echo "$OUT" | sed -n 's/.*elapsed_ns=\([0-9][0-9]*\).*/\1/p')
[ -n "$ENS" ] && [ "$ENS" -gt 0 ]

echo "$OUT" | grep -q "sink="

echo "E09 OK"
