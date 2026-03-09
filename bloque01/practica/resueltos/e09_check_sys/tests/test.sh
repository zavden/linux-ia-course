#!/bin/bash
set -euo pipefail

make -s clean
make -s all

if ./build/main >/tmp/e09.out 2>/tmp/e09.err; then
  echo "E09 debía fallar"
  exit 1
fi

grep -q "falló" /tmp/e09.err
rm -f /tmp/e09.out /tmp/e09.err

echo "E09 OK"
