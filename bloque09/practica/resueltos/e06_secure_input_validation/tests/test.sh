#!/bin/bash
set -euo pipefail

make -s clean
make -s all

OUT=$(./build/main alice_01 443)
echo "$OUT" | grep -q "user_ok=1"
echo "$OUT" | grep -q "port_ok=1"
echo "$OUT" | grep -q "port=443"

set +e
BAD=$(./build/main "../root" 70000 2>/dev/null)
RC=$?
set -e

[ "$RC" -ne 0 ]
echo "$BAD" | grep -q "user_ok=0"
echo "$BAD" | grep -q "port_ok=0"

echo "E06 OK"
