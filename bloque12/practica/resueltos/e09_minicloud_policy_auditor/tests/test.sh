#!/bin/bash
set -euo pipefail

make -s clean
make -s all
GOOD=$(./build/main tests/data/policy.good.conf)
echo "$GOOD" | grep -q "status=OK"

set +e
BAD=$(./build/main tests/data/policy.bad.conf 2>/dev/null)
RC=$?
set -e

[ "$RC" -ne 0 ]
echo "$BAD" | grep -q "status=CRIT"

echo "E09 OK"
