#!/bin/bash
set -euo pipefail

make -s clean
make -s all

mkdir -p tree/a/b
printf "x" > tree/a/b/file.txt

OUT=$(./build/main tree)
echo "$OUT" | grep -q "tree/a/b/file.txt"

rm -rf tree
echo "E09 OK"
