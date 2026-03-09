#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "padre_recibio=respuesta: hola_hijo"

echo "E06 OK"
