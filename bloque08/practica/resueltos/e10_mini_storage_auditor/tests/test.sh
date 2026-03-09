#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main . tests/data/quota.sample)

echo "$OUT" | grep -q "quota_over_soft=2"
echo "$OUT" | grep -E -q "status=(WARN|CRIT)"

echo "E10 OK"
