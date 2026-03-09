#!/bin/bash
set -euo pipefail

make -s clean
make -s all

OUT1=$(./build/main)
echo "$OUT1" | grep -q "matched=1"
echo "$OUT1" | grep -q "prefix=/api"
echo "$OUT1" | grep -q "backend=api"

OUT2=$(./build/main tests/data/routes.sample /unknown)
echo "$OUT2" | grep -q "prefix=/"
echo "$OUT2" | grep -q "backend=web"

echo "E09 OK"
