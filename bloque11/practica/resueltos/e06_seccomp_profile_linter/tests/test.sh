#!/bin/bash
set -euo pipefail

make -s clean
make -s all
GOOD=$(./build/main tests/data/seccomp.good.json)
echo "$GOOD" | grep -q "secure=1"

set +e
BAD=$(./build/main tests/data/seccomp.bad.json 2>/dev/null)
RC=$?
set -e

[ "$RC" -ne 0 ]
echo "$BAD" | grep -q "secure=0"

echo "E06 OK"
