#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "path=./archivo_que_no_existe.txt"
echo "$OUT" | grep -q "err_name=ENOENT"

echo "E03 OK"
