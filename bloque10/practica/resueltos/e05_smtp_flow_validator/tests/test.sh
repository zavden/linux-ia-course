#!/bin/bash
set -euo pipefail

make -s clean
make -s all
GOOD=$(./build/main tests/data/smtp.good.session)

echo "$GOOD" | grep -q "valid=1"
echo "$GOOD" | grep -q "rcpt=2"
echo "$GOOD" | grep -q "body_lines=3"

set +e
BAD=$(./build/main tests/data/smtp.bad.session 2>/dev/null)
RC=$?
set -e

[ "$RC" -ne 0 ]
echo "$BAD" | grep -q "valid=0"

echo "E05 OK"
