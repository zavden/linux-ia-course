#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "hijo_recibio=HOLA PIPE"

echo "E05 OK"
