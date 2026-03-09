#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main .)

echo "$OUT" | grep -q "path=."
TK=$(echo "$OUT" | sed -n 's/.*total_kib=\([0-9][0-9]*\).*/\1/p')
AK=$(echo "$OUT" | sed -n 's/.*avail_kib=\([0-9][0-9]*\).*/\1/p')

[ -n "$TK" ] && [ "$TK" -gt 0 ]
[ -n "$AK" ]

echo "E01 OK"
