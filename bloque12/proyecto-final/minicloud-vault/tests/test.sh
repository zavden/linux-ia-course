#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "valid=4"
echo "$OUT" | grep -q "invalid=0"
echo "$OUT" | grep -q "rotatable=3"
echo "$OUT" | grep -q "ready=1"

GET=$(./build/main --get runner_api_key tests/data/secrets.sample)
echo "$GET" | grep -q "found=1"
echo "$GET" | grep -q "ref=ref-run-501"

echo "VAULT OK"
