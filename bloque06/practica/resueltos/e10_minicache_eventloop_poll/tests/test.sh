#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "resp0=OK|VALUE Ana|"
echo "$OUT" | grep -q "resp1=OK|OK|NULL|"

echo "E10 OK"
