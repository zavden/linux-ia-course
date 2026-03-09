#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "esperando"
echo "$OUT" | grep -q "recolectado code=42"

echo "E02 OK"
