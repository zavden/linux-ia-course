#!/bin/bash
set -euo pipefail

make -s clean
make -s all

OUT=$(./build/main -u hola mundo)
echo "$OUT" | grep -q "HOLA"
echo "$OUT" | grep -q "MUNDO"

OUT2=$(./build/main -- -u literal)
echo "$OUT2" | grep -q -- "-u"

echo "E02 OK"
