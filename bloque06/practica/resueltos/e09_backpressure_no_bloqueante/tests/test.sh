#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "blocked=1"

WB=$(echo "$OUT" | sed -n 's/.*wrote_before=\([0-9][0-9]*\).*/\1/p')
WD=$(echo "$OUT" | sed -n 's/.*wrote_after=\([0-9][0-9]*\).*/\1/p')
DR=$(echo "$OUT" | sed -n 's/.*drained=\([0-9][0-9]*\).*/\1/p')

[ -n "$WB" ] && [ "$WB" -gt 0 ]
[ -n "$WD" ] && [ "$WD" -gt 0 ]
[ -n "$DR" ] && [ "$DR" -gt 0 ]

echo "E09 OK"
