#!/bin/bash
set -euo pipefail

make -s clean
make -s all
./build/main | grep -q "C01 plantilla lista"

echo "C01 plantilla OK"
