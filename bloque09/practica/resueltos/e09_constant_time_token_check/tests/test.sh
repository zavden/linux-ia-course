#!/bin/bash
set -euo pipefail

make -s clean
make -s all

GOOD=$(./build/main A1B2C3D4E5F60708)
echo "$GOOD" | grep -q "len_ok=1"
echo "$GOOD" | grep -q "ct_match=1"
echo "$GOOD" | grep -q "naive_match=1"

set +e
BAD=$(./build/main A1B2C3D4E5F60709 2>/dev/null)
RC=$?
set -e

[ "$RC" -ne 0 ]
echo "$BAD" | grep -q "ct_match=0"

echo "E09 OK"
