#!/bin/bash
set -euo pipefail

make -s clean
make -s all

if ./build/main >/tmp/e08.out 2>/tmp/e08.err; then
  echo "E08 debía fallar al abrir archivo inexistente"
  exit 1
fi

grep -q "fopen" /tmp/e08.err
rm -f /tmp/e08.out /tmp/e08.err

echo "E08 OK"
