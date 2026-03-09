#!/bin/bash
set -euo pipefail

make -s clean
make -s all

WARN_OUT=$(./build/main tests/data/health.warn.sample)
echo "$WARN_OUT" | grep -q "final=WARN"

CRIT_OUT=$(./build/main tests/data/health.crit.sample)
echo "$CRIT_OUT" | grep -q "final=CRIT"

echo "E10 OK"
