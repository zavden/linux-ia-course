#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main tests/data/quota.sample)

echo "$OUT" | grep -q "users=4"
echo "$OUT" | grep -q "over_soft=2"
echo "$OUT" | grep -q "over_hard=2"

echo "E08 OK"
