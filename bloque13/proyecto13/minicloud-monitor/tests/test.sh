#!/bin/bash
set -euo pipefail

PORT=19084
make -s clean
make -s all

./build/main --serve "$PORT" >/tmp/b13-monitor.log 2>&1 &
PID=$!
trap 'kill "$PID" 2>/dev/null || true' EXIT
sleep 0.4

H=$(./build/main --client 127.0.0.1 "$PORT" "HEALTH")
echo "$H" | grep -q "OK"

A=$(./build/main --client 127.0.0.1 "$PORT" "EVENT gateway ok 20")
echo "$A" | grep -q "ACK"

B=$(./build/main --client 127.0.0.1 "$PORT" "EVENT runner err 140")
echo "$B" | grep -q "ACK"

R=$(./build/main --client 127.0.0.1 "$PORT" "REPORT")
echo "$R" | grep -q "REPORT"
echo "$R" | grep -q "total=2"
echo "$R" | grep -q "errors=1"

echo "MONITOR13 OK"
