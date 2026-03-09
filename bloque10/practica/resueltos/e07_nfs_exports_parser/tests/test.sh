#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "exports=2"
echo "$OUT" | grep -q "hosts=3"
echo "$OUT" | grep -q "rw_hosts=2"
echo "$OUT" | grep -q "ro_hosts=1"
echo "$OUT" | grep -q "no_root_squash=1"

echo "E07 OK"
