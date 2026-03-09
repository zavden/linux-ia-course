#!/bin/bash
set -euo pipefail

make -s clean
make -s all
GOOD=$(./build/main)

echo "$GOOD" | grep -q "valid=1"
echo "$GOOD" | grep -q "host=api.example.com"
echo "$GOOD" | grep -q "port=8443"
echo "$GOOD" | grep -q "path=/v1/health"

set +e
BAD=$(./build/main http://api.example.com/v1 2>/dev/null)
RC=$?
set -e

[ "$RC" -ne 0 ]
echo "$BAD" | grep -q "valid=0"

echo "E04 OK"
