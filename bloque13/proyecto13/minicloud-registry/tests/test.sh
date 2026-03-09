#!/bin/bash
set -euo pipefail

PORT=19081
make -s clean
make -s all

./build/main --serve "$PORT" tests/data/routes.sample >/tmp/b13-registry.log 2>&1 &
PID=$!
trap 'kill "$PID" 2>/dev/null || true' EXIT
sleep 0.4

OUT1=$(./build/main --client 127.0.0.1 "$PORT" "HEALTH")
echo "$OUT1" | grep -q "OK"

OUT2=$(./build/main --client 127.0.0.1 "$PORT" "RESOLVE /run/job")
echo "$OUT2" | grep -q "BACKEND runner 127.0.0.1 19083"

OUT3=$(./build/main --client 127.0.0.1 "$PORT" "SNAPSHOT")
echo "$OUT3" | grep -q "total=3"
echo "$OUT3" | grep -q "up=2"
echo "$OUT3" | grep -q "down=1"

echo "REGISTRY13 OK"
