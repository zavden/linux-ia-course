#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "pending_usr1=1"
echo "$OUT" | grep -q "got_usr1_while_blocked=0"
echo "$OUT" | grep -q "got_usr1_after_unblock=1"

echo "E08 OK"
