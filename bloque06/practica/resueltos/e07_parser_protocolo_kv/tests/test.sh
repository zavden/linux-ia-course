#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "resp0=OK"
echo "$OUT" | grep -q "resp1=VALUE ana"
echo "$OUT" | grep -q "resp2=OK"
echo "$OUT" | grep -q "resp3=NULL"
echo "$OUT" | grep -q "resp4=ERR"

echo "E07 OK"
