#!/bin/bash
set -euo pipefail

PORT=29081
make -s clean
make -s all

./build/main --serve "$PORT" tests/data/routes.sample >/tmp/b14-registry.log 2>&1 &
PID=$!
trap 'kill "$PID" 2>/dev/null || true' EXIT
sleep 0.4

H=$(./build/main --client 127.0.0.1 "$PORT" GET /health)
echo "$H" | grep -q "HTTP/1.1 200"
echo "$H" | grep -q "OK"

R=$(./build/main --client 127.0.0.1 "$PORT" GET '/resolve?path=/run/job-a')
echo "$R" | grep -q "HTTP/1.1 200"
echo "$R" | grep -q "BACKEND runner"

S=$(./build/main --client 127.0.0.1 "$PORT" GET /snapshot)
echo "$S" | grep -q "total=3"

echo "REGISTRY14 OK"
