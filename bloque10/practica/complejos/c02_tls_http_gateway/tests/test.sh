#!/bin/bash
set -euo pipefail

make -s clean
make -s all
./build/main | grep -q "C02 plantilla lista"

echo "C02 plantilla OK"
