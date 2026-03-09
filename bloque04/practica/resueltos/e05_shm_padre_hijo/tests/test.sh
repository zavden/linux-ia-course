#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "hijo_lee=mensaje_padre"
echo "$OUT" | grep -q "padre_lee=respuesta_hijo"

echo "E05 OK"
