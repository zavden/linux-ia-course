#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main tests/data/mounts.sample tests/data/lvs.sample tests/data/quota.sample)

echo "$OUT" | grep -q '"mounts":2'
echo "$OUT" | grep -q '"lvs":2'
echo "$OUT" | grep -q '"quota_over_soft":1'

echo "E09 OK"
