#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main abc)

echo "$OUT" | grep -q "msg=abc"
echo "$OUT" | grep -q "sha256=ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"

echo "E04 OK"
