#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "mnt=1"
echo "$OUT" | grep -q "uts=1"
echo "$OUT" | grep -q "pid=1"
echo "$OUT" | grep -q "net=1"
echo "$OUT" | grep -q "count=4"

echo "E03 OK"
