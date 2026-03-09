#!/bin/bash
set -euo pipefail

make -s clean
make -s all
./build/main | grep -q "C03 plantilla lista"

echo "C03 plantilla OK"
