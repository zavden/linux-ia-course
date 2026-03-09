#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "expected=1600000"
echo "$OUT" | grep -q "observed=1600000"
echo "$OUT" | grep -q "mutex_ok=1"

echo "E04 OK"
