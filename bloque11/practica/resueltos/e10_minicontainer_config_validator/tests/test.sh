#!/bin/bash
set -euo pipefail

make -s clean
make -s all
GOOD=$(./build/main tests/data/minicontainer.good.conf)
echo "$GOOD" | grep -q "valid=1"
echo "$GOOD" | grep -q "present=6"

set +e
BAD=$(./build/main tests/data/minicontainer.bad.conf 2>/dev/null)
RC=$?
set -e

[ "$RC" -ne 0 ]
echo "$BAD" | grep -q "valid=0"

echo "E10 OK"
