#!/bin/bash
set -euo pipefail

make -s clean
make -s all
GOOD=$(./build/main tests/data/tls.good.conf)

echo "$GOOD" | grep -q "strong=1"

set +e
BAD=$(./build/main tests/data/tls.bad.conf 2>/dev/null)
RC=$?
set -e

[ "$RC" -ne 0 ]
echo "$BAD" | grep -q "strong=0"

echo "E03 OK"
