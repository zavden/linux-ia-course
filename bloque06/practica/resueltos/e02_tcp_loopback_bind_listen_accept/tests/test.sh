#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "server_read=hola_tcp"
echo "$OUT" | grep -q "child_read=ok:hola_tcp"
echo "$OUT" | grep -q "port="

echo "E02 OK"
