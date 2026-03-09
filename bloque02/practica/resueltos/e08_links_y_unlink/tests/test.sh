#!/bin/bash
set -euo pipefail

make -s clean
make -s all

OUT=$(./build/main base.txt hard.txt sym.txt)

echo "$OUT" | grep -q "hard_data=Hola Mundo"
echo "$OUT" | grep -q "symlink_open_error"

rm -f base.txt hard.txt sym.txt
echo "E08 OK"
