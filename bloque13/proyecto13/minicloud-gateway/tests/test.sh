#!/bin/bash
set -euo pipefail

REG_PORT=19081
VAULT_PORT=19082
RUN_PORT=19083
MON_PORT=19084

make -s clean
make -s all

make -s -C ../minicloud-registry all
make -s -C ../minicloud-vault all
make -s -C ../minicloud-runner all
make -s -C ../minicloud-monitor all

../minicloud-registry/build/main --serve "$REG_PORT" tests/data/routes.sample >/tmp/b13-gw-reg.log 2>&1 &
PID_REG=$!
../minicloud-vault/build/main --serve "$VAULT_PORT" tests/data/secrets.sample >/tmp/b13-gw-vault.log 2>&1 &
PID_VAULT=$!
../minicloud-runner/build/main --serve "$RUN_PORT" tests/data/capacity.sample >/tmp/b13-gw-run.log 2>&1 &
PID_RUN=$!
../minicloud-monitor/build/main --serve "$MON_PORT" >/tmp/b13-gw-mon.log 2>&1 &
PID_MON=$!

cleanup() {
  kill "$PID_REG" 2>/dev/null || true
  kill "$PID_VAULT" 2>/dev/null || true
  kill "$PID_RUN" 2>/dev/null || true
  kill "$PID_MON" 2>/dev/null || true
}
trap cleanup EXIT

sleep 0.8

OUT=$(./build/main --request /run/job-api \
  --registry 127.0.0.1:$REG_PORT \
  --vault 127.0.0.1:$VAULT_PORT \
  --runner 127.0.0.1:$RUN_PORT \
  --monitor 127.0.0.1:$MON_PORT)

echo "$OUT" | grep -q "status=ACCEPT"

REP=$(../minicloud-monitor/build/main --client 127.0.0.1 "$MON_PORT" "REPORT")
echo "$REP" | grep -q "REPORT"
echo "$REP" | grep -q "total=1"
echo "$REP" | grep -q "errors=0"

echo "GATEWAY13 OK"
