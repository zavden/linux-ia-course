#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "got_left=ping"
echo "$OUT" | grep -q "got_right=pong"

echo "E01 OK"
