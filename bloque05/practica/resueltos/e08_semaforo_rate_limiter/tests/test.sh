#!/bin/bash
set -euo pipefail

make -s clean
make -s all
OUT=$(./build/main)

echo "$OUT" | grep -q "threads_done=10"
echo "$OUT" | grep -q "capacity=3"

MAX_ACTIVE=$(echo "$OUT" | sed -n 's/.*max_active=\([0-9][0-9]*\).*/\1/p')
if [ -z "$MAX_ACTIVE" ]; then
  echo "No se pudo parsear max_active"
  exit 1
fi

if [ "$MAX_ACTIVE" -gt 3 ]; then
  echo "max_active excede capacidad: $MAX_ACTIVE"
  exit 1
fi

echo "E08 OK"
