#!/bin/bash
set -euo pipefail

PASS=0
FAIL=0

pass() { echo "  ✅ PASS: $1"; ((PASS++)); }
fail() { echo "  ❌ FAIL: $1"; ((FAIL++)); }

echo "=== Tests Proyecto 00 (build-lab) ==="

SCRIPT="./solucion/build_lab.sh"

if [ ! -x "$SCRIPT" ]; then
    chmod +x "$SCRIPT"
fi

# 1. Probar que pide argumentos
OUTPUT=$($SCRIPT 2>&1 || true)
if echo "$OUTPUT" | grep -q "Uso:"; then
    pass "Valida argumentos de entrada"
else
    fail "No valida argumentos correctamente"
fi

# 2. Probar contra el ej01 solucion (debe pasar)
# Copiamos la solucion al src de ej01 para asegurar que pase test
cp ../ej01/solucion/src/main.c ../ej01/src/main.c
OUTPUT=$($SCRIPT ../ej01/ || true)
if echo "$OUTPUT" | grep -q "Fedora: ✅ PASSED" && echo "$OUTPUT" | grep -q "Debian: ✅ PASSED"; then
    pass "Test exitoso en solucion ej01 para ambas distros"
else
    fail "Fallo al procesar ej01:\n$OUTPUT"
fi

echo "Resultado test completo: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ] || exit 1
