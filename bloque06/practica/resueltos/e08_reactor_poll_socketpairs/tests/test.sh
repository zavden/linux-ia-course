#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "handled=3"
echo "$OUT" | grep -q "c0=HOLA"
echo "$OUT" | grep -q "c1=MUNDO"
echo "$OUT" | grep -q "c2=REACTOR"

echo "E08 OK"
