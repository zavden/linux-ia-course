#!/bin/bash
set -euo pipefail

PORT=29082
make -s clean
make -s all

./build/main --serve "$PORT" tests/data/secrets.sample >/tmp/b14-vault.log 2>&1 &
PID=$!
trap 'kill "$PID" 2>/dev/null || true' EXIT
sleep 0.4

H=$(./build/main --client 127.0.0.1 "$PORT" GET /health)
echo "$H" | grep -q "HTTP/1.1 200"

S=$(./build/main --client 127.0.0.1 "$PORT" GET /secret/runner_api_key)
echo "$S" | grep -q "HTTP/1.1 200"
echo "$S" | grep -q "SECRET ref-run-001"

N=$(./build/main --client 127.0.0.1 "$PORT" GET /secret/deprecated_key)
echo "$N" | grep -q "HTTP/1.1 404"

T=$(./build/main --client 127.0.0.1 "$PORT" GET /stats)
echo "$T" | grep -q "active=2"

echo "VAULT14 OK"
