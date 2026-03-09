#!/bin/bash
set -euo pipefail

make -s clean
make -s all

printf "ABCD\n" > tfile.txt
./build/main tfile.txt
FIRST=$(head -c 1 tfile.txt)
[ "$FIRST" = "X" ]

rm -f tfile.txt
echo "E03 OK"
