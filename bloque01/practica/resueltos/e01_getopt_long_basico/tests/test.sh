#!/bin/bash
set -euo pipefail

make -s clean
make -s all

OUT=$(./build/main --verbose --number=7)
echo "$OUT" | grep -q "verbose=ON"
echo "$OUT" | grep -q "number=7"

if ./build/main --number=abc >/dev/null 2>&1; then
  echo "Debía fallar con --number inválido"
  exit 1
fi

echo "E01 OK"
