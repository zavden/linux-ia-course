#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "sys_byte_ms="
echo "$OUT" | grep -q "stdio_byte_ms="
cmp -s bench_src.dat bench_sys.dat
cmp -s bench_src.dat bench_stdio.dat

rm -f bench_src.dat bench_sys.dat bench_stdio.dat
echo "E03 OK"
