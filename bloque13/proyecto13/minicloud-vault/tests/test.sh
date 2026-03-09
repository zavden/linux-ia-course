#!/bin/bash
set -euo pipefail

PORT=19082
make -s clean
make -s all

./build/main --serve "$PORT" tests/data/secrets.sample >/tmp/b13-vault.log 2>&1 &
PID=$!
trap 'kill "$PID" 2>/dev/null || true' EXIT
sleep 0.4

H=$(./build/main --client 127.0.0.1 "$PORT" "HEALTH")
echo "$H" | grep -q "OK"

S=$(./build/main --client 127.0.0.1 "$PORT" "GET runner_api_key")
echo "$S" | grep -q "SECRET ref-run-001"

M=$(./build/main --client 127.0.0.1 "$PORT" "GET deprecated_key")
echo "$M" | grep -q "NOTFOUND"

T=$(./build/main --client 127.0.0.1 "$PORT" "STATS")
echo "$T" | grep -q "total=3"
echo "$T" | grep -q "active=2"
echo "$T" | grep -q "revoked=1"

echo "VAULT13 OK"
