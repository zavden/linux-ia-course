#!/bin/bash
set -euo pipefail

PORT=29083
make -s clean
make -s all

./build/main --serve "$PORT" tests/data/capacity.sample >/tmp/b14-runner.log 2>&1 &
PID=$!
trap 'kill "$PID" 2>/dev/null || true' EXIT
sleep 0.4

H=$(./build/main --client 127.0.0.1 "$PORT" GET /health)
echo "$H" | grep -q "HTTP/1.1 200"

A=$(./build/main --client 127.0.0.1 "$PORT" GET '/run?job=jobA&cpu=500&mem=512&timeout=60&secret=ref-1')
echo "$A" | grep -q "HTTP/1.1 200"
echo "$A" | grep -q "ACCEPT"

B=$(./build/main --client 127.0.0.1 "$PORT" GET '/run?job=jobB&cpu=2600&mem=4096&timeout=60&secret=ref-2')
echo "$B" | grep -q "HTTP/1.1 409"
echo "$B" | grep -q "REJECT no_capacity"

S=$(./build/main --client 127.0.0.1 "$PORT" GET /stats)
echo "$S" | grep -q "accepted=1"
echo "$S" | grep -q "rejected=1"

echo "RUNNER14 OK"
