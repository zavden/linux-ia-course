#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "shares=3"
echo "$OUT" | grep -q "writable=2"
echo "$OUT" | grep -q "guest_ok=1"
echo "$OUT" | grep -q "browseable_no=1"

echo "E08 OK"
