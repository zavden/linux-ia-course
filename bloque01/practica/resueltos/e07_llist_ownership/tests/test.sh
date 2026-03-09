#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "lista creada"
echo "$OUT" | grep -q "ok"

echo "E07 OK"
