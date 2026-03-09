#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "total=6"
echo "$OUT" | grep -q "errors=2"
echo "$OUT" | grep -q "avg_ms=76.67"
echo "$OUT" | grep -q "err_pct=33.33"

echo "E05 OK"
