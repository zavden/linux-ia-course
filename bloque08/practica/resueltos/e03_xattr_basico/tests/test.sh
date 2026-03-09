#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -E -q "mode=(xattr|sidecar)"
echo "$OUT" | grep -q "value=hola_xattr"

echo "E03 OK"
