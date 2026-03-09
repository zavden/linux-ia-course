#!/bin/bash
set -euo pipefail

make -s clean
make -s all

./build/main > e07.log 2>&1 &
PID=$!
sleep 0.4
kill -USR1 "$PID"
sleep 0.2
kill -USR1 "$PID"
sleep 0.3
kill -TERM "$PID"
wait "$PID"

grep -q "usr1_count=2" e07.log
grep -q "stop=1" e07.log

rm -f e07.log
echo "E07 OK"
