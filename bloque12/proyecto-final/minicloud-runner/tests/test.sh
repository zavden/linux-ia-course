#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "valid=3"
echo "$OUT" | grep -q "invalid=0"
echo "$OUT" | grep -q "admitted=3"
echo "$OUT" | grep -q "ready=1"

echo "RUNNER OK"
