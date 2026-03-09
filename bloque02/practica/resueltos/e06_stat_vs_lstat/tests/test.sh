#!/bin/bash
set -euo pipefail

make -s clean
make -s all

echo "hola" > base.txt
ln -sf base.txt enlace.sym

OUT=$(./build/main enlace.sym)
echo "$OUT" | grep -q "type_stat=regular"
echo "$OUT" | grep -q "type_lstat=symlink"

rm -f base.txt enlace.sym
echo "E06 OK"
