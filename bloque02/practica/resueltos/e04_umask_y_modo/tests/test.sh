#!/bin/bash
set -euo pipefail

make -s clean
make -s all

umask 077
OUT=$(./build/main demo_mode.txt 0644)
echo "$OUT" | grep -q "mode=644"

rm -f demo_mode.txt
echo "E04 OK"
