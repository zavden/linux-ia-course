#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "my_memcpy_ms="
echo "$OUT" | grep -q "libc_memcpy_ms="

echo "E10 OK"
