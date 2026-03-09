#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "valid=5"
echo "$OUT" | grep -q "invalid=0"
echo "$OUT" | grep -q "up=4"
echo "$OUT" | grep -q "secure=2"
echo "$OUT" | grep -q "ready=1"

echo "REGISTRY OK"
