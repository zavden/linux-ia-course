#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "valid=4"
echo "$OUT" | grep -q "invalid=1"
echo "$OUT" | grep -q "https=2"

echo "E01 OK"
