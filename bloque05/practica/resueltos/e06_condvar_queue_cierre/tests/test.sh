#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "produced=50"
echo "$OUT" | grep -q "consumed=50"
echo "$OUT" | grep -q "queue_count=0"

echo "E06 OK"
