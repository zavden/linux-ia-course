#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "greet=220"
echo "$OUT" | grep -q "final=250"
echo "$OUT" | grep -q "caps=2"
echo "$OUT" | grep -q "starttls=1"

echo "E06 OK"
