#!/bin/bash
set -euo pipefail

make -s clean
make -s all

OUT=$(./build/main)
echo "$OUT" | grep -q "valid=3"
echo "$OUT" | grep -q "invalid=1"
echo "$OUT" | grep -q "admin=2"

echo "E10 OK"
