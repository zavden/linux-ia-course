#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -E -q "method=(inotify|polling)"
echo "$OUT" | grep -q "detected=1"

echo "E06 OK"
