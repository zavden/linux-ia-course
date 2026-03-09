#!/bin/bash
set -euo pipefail

make -s clean
make -s all

cat > meminfo_sample.txt <<'S'
MemTotal:       1000000 kB
MemFree:         200000 kB
Buffers:          50000 kB
Cached:          150000 kB
S

OUT=$(./build/main meminfo_sample.txt)
echo "$OUT" | grep -q "total_kb=1000000"
echo "$OUT" | grep -q "used_kb=600000"

rm -f meminfo_sample.txt
echo "E09 OK"
