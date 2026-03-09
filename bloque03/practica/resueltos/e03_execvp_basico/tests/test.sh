#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main echo hola)

echo "$OUT" | grep -q "hola"
echo "$OUT" | grep -q "child_exit=0"

echo "E03 OK"
