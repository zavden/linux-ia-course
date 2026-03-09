#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "first_try=eagain"
echo "$OUT" | grep -q "second_read=abc"
echo "$OUT" | grep -q "bytes=3"

echo "E03 OK"
