#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main . /tmp)

echo "$OUT" | grep -q "fs path=."
echo "$OUT" | grep -q "ok=2"
echo "$OUT" | grep -q "fail=0"

echo "E05 OK"
