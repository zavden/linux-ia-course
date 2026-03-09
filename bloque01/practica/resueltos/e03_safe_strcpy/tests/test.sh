#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "truncated=yes"
echo "$OUT" | grep -q "dst='LinuxCo'"

echo "E03 OK"
