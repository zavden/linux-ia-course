#!/bin/bash
set -euo pipefail

make -s clean
make -s all

mkdir -p fake_proc/100 fake_proc/200
cat > meminfo.txt <<'M'
MemTotal:       800000 kB
MemFree:        100000 kB
Buffers:         50000 kB
Cached:         150000 kB
M

cat > fake_proc/100/status <<'S1'
Name:\tbash
VmRSS:\t1234 kB
S1

cat > fake_proc/200/status <<'S2'
Name:\tnginx
VmRSS:\t5678 kB
S2

OUT=$(./build/main meminfo.txt fake_proc)

echo "$OUT" | grep -q "mem_used_kb=500000"
echo "$OUT" | grep -q "pid=100"
echo "$OUT" | grep -q "pid=200"

rm -rf fake_proc meminfo.txt
echo "E10 OK"
