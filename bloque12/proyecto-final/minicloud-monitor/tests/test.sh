#!/bin/bash
set -euo pipefail

make -s clean
make -s all
set +e
OUT=$(./build/main 2>/dev/null)
RC=$?
set -e

[ "$RC" -ne 0 ]
echo "$OUT" | grep -q "req=6"
echo "$OUT" | grep -q "err=2"
echo "$OUT" | grep -q "status=CRIT"

echo "MONITOR OK"
