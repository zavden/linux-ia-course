#!/bin/bash
set -euo pipefail

make -s clean
make -s all

OK=$(./build/main tests/data/readiness.ok.sample)
echo "$OK" | grep -q "status=OK"
echo "$OK" | grep -q "ready=1"

set +e
WARN=$(./build/main tests/data/readiness.warn.sample 2>/dev/null)
RC=$?
set -e

[ "$RC" -ne 0 ]
echo "$WARN" | grep -q "status=WARN"
echo "$WARN" | grep -q "ready=0"

echo "E10 OK"
