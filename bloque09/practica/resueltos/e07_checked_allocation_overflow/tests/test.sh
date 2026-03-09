#!/bin/bash
set -euo pipefail

make -s clean
make -s all

OUT=$(./build/main 128 64)
echo "$OUT" | grep -q "bytes=8192"
echo "$OUT" | grep -q "overflow=0"

set +e
BAD=$(./build/main 4294967296 4294967296 2>/dev/null)
RC=$?
set -e

[ "$RC" -ne 0 ]
echo "$BAD" | grep -q "overflow=1"

echo "E07 OK"
