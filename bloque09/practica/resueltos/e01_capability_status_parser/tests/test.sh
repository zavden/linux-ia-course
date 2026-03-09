#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main tests/data/status.sample)

echo "$OUT" | grep -q "cap_eff=0x2400"
echo "$OUT" | grep -q "bind=1"
echo "$OUT" | grep -q "raw=1"

echo "E01 OK"
