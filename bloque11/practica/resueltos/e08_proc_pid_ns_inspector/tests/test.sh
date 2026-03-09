#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "ns_total=8"
echo "$OUT" | grep -q "unique_ids=8"
echo "$OUT" | grep -q "has_user=1"
echo "$OUT" | grep -q "has_time=1"

echo "E08 OK"
