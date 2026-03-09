#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main tests/data/sestatus.sample tests/data/aa-status.sample)

echo "$OUT" | grep -q "selinux_mode=enforcing"
echo "$OUT" | grep -q "aa_enforce=2"
echo "$OUT" | grep -q "aa_complain=1"

echo "E03 OK"
