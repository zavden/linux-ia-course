#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q '"total":5'
echo "$OUT" | grep -q '"up":4'
echo "$OUT" | grep -q '"down":1'
echo "$OUT" | grep -q '"invalid":1'

echo "E02 OK"
