#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "before_soft="
echo "$OUT" | grep -q "after_soft="

echo "E06 OK"
