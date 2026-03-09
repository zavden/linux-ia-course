#!/bin/bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

REG_OUT=$("$ROOT_DIR/minicloud-registry/build/main" "$ROOT_DIR/tests/data/contracts.sample")
GW_OUT=$("$ROOT_DIR/minicloud-gateway/build/main" "$ROOT_DIR/tests/data/routes.sample" /api/v1/users)
MON_OUT=$("$ROOT_DIR/minicloud-monitor/build/main" "$ROOT_DIR/tests/data/metrics.sample" "$ROOT_DIR/tests/data/events.sample")
VAULT_OUT=$("$ROOT_DIR/minicloud-vault/build/main" "$ROOT_DIR/tests/data/secrets.sample")
RUN_OUT=$("$ROOT_DIR/minicloud-runner/build/main" "$ROOT_DIR/tests/data/jobs.sample" "$ROOT_DIR/tests/data/runner.capacity")

echo "$REG_OUT" | grep -q 'valid=5'
echo "$GW_OUT" | grep -q 'plan=ok'
echo "$MON_OUT" | grep -E -q 'status=(WARN|CRIT)'
echo "$VAULT_OUT" | grep -q 'valid=4'
echo "$RUN_OUT" | grep -q 'admitted=3'

echo "STACK OK"
