#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "total=5"
echo "$OUT" | grep -q "ok=3"
echo "$OUT" | grep -q "warn=1"
echo "$OUT" | grep -q "crit=1"
echo "$OUT" | grep -q "global=CRIT"

echo "E04 OK"
