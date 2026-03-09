#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main tests/data/acl.sample)

echo "$OUT" | grep -q "users=2"
echo "$OUT" | grep -q "groups=2"
echo "$OUT" | grep -q "defaults=3"

echo "E04 OK"
