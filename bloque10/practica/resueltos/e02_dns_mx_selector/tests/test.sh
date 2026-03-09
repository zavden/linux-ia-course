#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "mx_count=3"
echo "$OUT" | grep -q "best_pref=10"
echo "$OUT" | grep -q "best_host=mail1.example.com"

echo "E02 OK"
