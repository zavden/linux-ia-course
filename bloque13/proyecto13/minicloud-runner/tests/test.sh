#!/bin/bash
set -euo pipefail

PORT=19083
make -s clean
make -s all

./build/main --serve "$PORT" tests/data/capacity.sample >/tmp/b13-runner.log 2>&1 &
PID=$!
trap 'kill "$PID" 2>/dev/null || true' EXIT
sleep 0.4

H=$(./build/main --client 127.0.0.1 "$PORT" "HEALTH")
echo "$H" | grep -q "OK"

A=$(./build/main --client 127.0.0.1 "$PORT" "RUN jobA 500 512 60 ref-1")
echo "$A" | grep -q "ACCEPT"

B=$(./build/main --client 127.0.0.1 "$PORT" "RUN jobB 2600 4096 60 ref-2")
echo "$B" | grep -q "REJECT no_capacity"

S=$(./build/main --client 127.0.0.1 "$PORT" "STATS")
echo "$S" | grep -q "accepted=1"
echo "$S" | grep -q "rejected=1"

echo "RUNNER13 OK"
