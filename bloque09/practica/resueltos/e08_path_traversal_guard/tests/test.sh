#!/bin/bash
set -euo pipefail

make -s clean
make -s all

OUT=$(./build/main /srv/vault docs/report.txt)
echo "$OUT" | grep -q "allowed=1"
echo "$OUT" | grep -q "normalized=docs/report.txt"

set +e
BAD=$(./build/main /srv/vault ../../etc/shadow 2>/dev/null)
RC=$?
set -e

[ "$RC" -ne 0 ]
echo "$BAD" | grep -q "allowed=0"

echo "E08 OK"
