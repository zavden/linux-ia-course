#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OK=$(./build/main)
echo "$OK" | grep -q "plan=ok"
echo "$OK" | grep -q "prefix=/api/v1"
echo "$OK" | grep -q "backend=api_v1"

set +e
BAD=$(./build/main tests/data/gateway.sample /admin/panel 2>/dev/null)
RC=$?
set -e

[ "$RC" -ne 0 ]
echo "$BAD" | grep -q "backend_down"

echo "E03 OK"
