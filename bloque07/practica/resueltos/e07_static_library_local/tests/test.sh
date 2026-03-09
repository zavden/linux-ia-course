#!/bin/bash
set -euo pipefail

make -s clean
make -s all

[ -f build/libtextstats.a ]
OUT=$(./build/main)

echo "$OUT" | grep -q "text=DebugMake"
echo "$OUT" | grep -q "vowels=4"
echo "$OUT" | grep -q "consonants=5"

echo "E07 OK"
