#!/bin/bash
set -euo pipefail

PORT=29084
make -s clean
make -s all

./build/main --serve "$PORT" >/tmp/b14-monitor.log 2>&1 &
PID=$!
trap 'kill "$PID" 2>/dev/null || true' EXIT
sleep 0.4

H=$(./build/main --client 127.0.0.1 "$PORT" GET /health)
echo "$H" | grep -q "HTTP/1.1 200"

A=$(./build/main --client 127.0.0.1 "$PORT" GET '/event?service=gateway&result=ok&lat_ms=20')
echo "$A" | grep -q "HTTP/1.1 202"

B=$(./build/main --client 127.0.0.1 "$PORT" GET '/event?service=runner&result=err&lat_ms=140')
echo "$B" | grep -q "HTTP/1.1 202"

R=$(./build/main --client 127.0.0.1 "$PORT" GET /report)
echo "$R" | grep -q "HTTP/1.1 200"
echo "$R" | grep -q "total=2"
echo "$R" | grep -q "errors=1"

echo "MONITOR14 OK"
