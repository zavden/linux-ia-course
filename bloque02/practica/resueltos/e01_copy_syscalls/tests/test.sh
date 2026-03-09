#!/bin/bash
set -euo pipefail

make -s clean
make -s all

printf 'abc123\nlinea2\n' > in.txt
./build/main in.txt out.txt
cmp -s in.txt out.txt

rm -f in.txt out.txt
echo "E01 OK"
