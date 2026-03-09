#!/bin/bash
set -euo pipefail

make -s clean
make -s all

mkdir -p tdir
printf "abc" > tdir/a.txt
ln -sf a.txt tdir/a.sym

OUT=$(./build/main tdir)
echo "$OUT" | grep -q "a.txt"
echo "$OUT" | grep -q "a.sym"

test -d tdir && rm -rf tdir
echo "E07 OK"
