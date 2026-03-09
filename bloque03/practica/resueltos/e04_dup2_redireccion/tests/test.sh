#!/bin/bash
set -euo pipefail

make -s clean
make -s all

./build/main salida.txt echo "hola dup2" >/tmp/e04.out

grep -q "hola dup2" salida.txt
grep -q "child_exit=0" /tmp/e04.out

rm -f salida.txt /tmp/e04.out
echo "E04 OK"
