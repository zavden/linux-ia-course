#!/bin/bash
set -euo pipefail

make -s clean
make -s all

for i in $(seq 1 200); do
  echo "linea"
done > in.txt
./build/main in.txt out.txt
cmp -s in.txt out.txt

rm -f in.txt out.txt
echo "E02 OK"
