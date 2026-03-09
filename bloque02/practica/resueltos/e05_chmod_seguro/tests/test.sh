#!/bin/bash
set -euo pipefail

make -s clean
make -s all

OUT=$(./build/main chmod_demo.txt 0751)
echo "$OUT" | grep -q "mode=751"

rm -f chmod_demo.txt
echo "E05 OK"
