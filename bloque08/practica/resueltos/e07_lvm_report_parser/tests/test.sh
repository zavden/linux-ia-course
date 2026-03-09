#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main tests/data/lvs.sample)

echo "$OUT" | grep -q "lvs=4"
echo "$OUT" | grep -q "thin=1"
TM=$(echo "$OUT" | sed -n 's/.*total_mib=\([0-9][0-9]*\).*/\1/p')
[ -n "$TM" ] && [ "$TM" -gt 10000 ]

echo "E07 OK"
