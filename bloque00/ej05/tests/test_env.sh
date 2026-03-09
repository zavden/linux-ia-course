#!/bin/bash
set -euo pipefail

BINARY="./build/main"
PASS=0
FAIL=0

pass() { echo "  ✅ PASS: $1"; ((PASS++)); }
fail() { echo "  ❌ FAIL: $1"; ((FAIL++)); }

echo "=== Tests Ejercicio 0.5 (Variables de Entorno) ==="

if [ ! -x "$BINARY" ]; then
    fail "El binario $BINARY no existe (corre make all primero)."
    exit 1
fi

# 1. Sin variables (Defaults)
OUTPUT=$(env -i "$BINARY")
if echo "$OUTPUT" | grep -q "development" && echo "$OUTPUT" | grep -q "8080"; then
    pass "Asigna los valores por defecto correctamente"
else
    fail "Fallo al asignar defaults. Output:\n$OUTPUT"
fi

# 2. Con APP_ENV custom
OUTPUT=$(env -i APP_ENV=staging "$BINARY")
if echo "$OUTPUT" | grep -q "staging"; then
    pass "Lee APP_ENV correctamente"
else
    fail "Fallo al leer APP_ENV. Output:\n$OUTPUT"
fi

# 3. Con APP_PORT custom
OUTPUT=$(env -i APP_PORT=3000 "$BINARY")
if echo "$OUTPUT" | grep -q "3000"; then
    pass "Lee APP_PORT y lo parsea correctamente"
else
    fail "Fallo al leer APP_PORT. Output:\n$OUTPUT"
fi

echo "Resultado test: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ] || exit 1
