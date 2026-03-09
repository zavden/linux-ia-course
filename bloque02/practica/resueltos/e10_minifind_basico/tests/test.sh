#!/bin/bash
set -euo pipefail

make -s clean
make -s all

mkdir -p tfind/a/b
printf "hola" > tfind/a/b/secreto.txt
printf "1234567890" > tfind/a/b/diez.bin
ln -sf secreto.txt tfind/a/b/link.sym

OUT1=$(./build/main tfind -name secreto.txt)
echo "$OUT1" | grep -q "secreto.txt"

OUT2=$(./build/main tfind -type d)
echo "$OUT2" | grep -q "tfind/a/b"

OUT3=$(./build/main tfind -size 10c)
echo "$OUT3" | grep -q "diez.bin"

rm -rf tfind
echo "E10 OK"
