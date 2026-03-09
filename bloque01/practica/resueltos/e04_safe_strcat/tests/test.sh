#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "dst='Hola Mundo '"
echo "$OUT" | grep -q "truncated=yes"

echo "E04 OK"
