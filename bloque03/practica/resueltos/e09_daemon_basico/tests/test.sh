#!/bin/bash
set -euo pipefail

make -s clean
make -s all

rm -f /tmp/e09_daemon.log
./build/main
sleep 4

grep -q "tick=3" /tmp/e09_daemon.log
rm -f /tmp/e09_daemon.log
echo "E09 OK"
