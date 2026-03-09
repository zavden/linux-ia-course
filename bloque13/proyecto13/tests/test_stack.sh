#!/bin/bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

REG_PORT=19181
VAULT_PORT=19182
RUN_PORT=19183
MON_PORT=19184

REG_PID=""
VAULT_PID=""
RUN_PID=""
MON_PID=""

cleanup() {
  [ -n "$REG_PID" ] && kill "$REG_PID" 2>/dev/null || true
  [ -n "$VAULT_PID" ] && kill "$VAULT_PID" 2>/dev/null || true
  [ -n "$RUN_PID" ] && kill "$RUN_PID" 2>/dev/null || true
  [ -n "$MON_PID" ] && kill "$MON_PID" 2>/dev/null || true
}
trap cleanup EXIT

"$ROOT_DIR/minicloud-registry/build/main" --serve "$REG_PORT" "$ROOT_DIR/tests/data/routes.sample" >/tmp/mc13-reg.log 2>&1 &
REG_PID=$!

"$ROOT_DIR/minicloud-vault/build/main" --serve "$VAULT_PORT" "$ROOT_DIR/tests/data/secrets.sample" >/tmp/mc13-vault.log 2>&1 &
VAULT_PID=$!

"$ROOT_DIR/minicloud-runner/build/main" --serve "$RUN_PORT" "$ROOT_DIR/tests/data/capacity.sample" >/tmp/mc13-run.log 2>&1 &
RUN_PID=$!

"$ROOT_DIR/minicloud-monitor/build/main" --serve "$MON_PORT" >/tmp/mc13-mon.log 2>&1 &
MON_PID=$!

sleep 1

REQ_OUT=$("$ROOT_DIR/minicloud-gateway/build/main" --request /run/job-api \
  --registry 127.0.0.1:$REG_PORT \
  --vault 127.0.0.1:$VAULT_PORT \
  --runner 127.0.0.1:$RUN_PORT \
  --monitor 127.0.0.1:$MON_PORT)

echo "$REQ_OUT" | grep -q "status=ACCEPT"

MON_OUT=$("$ROOT_DIR/minicloud-monitor/build/main" --client 127.0.0.1 "$MON_PORT" "REPORT")
echo "$MON_OUT" | grep -q "REPORT"
echo "$MON_OUT" | grep -q "total=1"

echo "STACK13 OK"
