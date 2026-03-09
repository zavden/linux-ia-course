#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main tests/data/mounts.sample tests/data/fstab.sample)

echo "$OUT" | grep -q "mounts=3"
echo "$OUT" | grep -q "fstab=3"
echo "$OUT" | grep -q "first_mount=/"
echo "$OUT" | grep -q "first_fstab=/"

echo "E02 OK"
