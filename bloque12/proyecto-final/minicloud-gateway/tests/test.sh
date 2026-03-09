#!/bin/bash
set -euo pipefail

make -s clean
make -s all
GOOD=$(./build/main)
echo "$GOOD" | grep -q "plan=ok"
echo "$GOOD" | grep -q "prefix=/api/v1"
echo "$GOOD" | grep -q "backend=api_v1"

set +e
BAD=$(./build/main tests/data/routes.sample /admin 2>/dev/null)
RC=$?
set -e

[ "$RC" -ne 0 ]
echo "$BAD" | grep -q "backend_down"

echo "GATEWAY OK"
