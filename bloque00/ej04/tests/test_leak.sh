#!/bin/bash
set -euo pipefail

PASS=0
FAIL=0

pass() { echo "  ✅ PASS: $1"; ((PASS++)); }
fail() { echo "  ❌ FAIL: $1"; ((FAIL++)); }

echo "=== Tests Ejercicio 0.4 (Valgrind) ==="

if [ ! -x "./build/main" ]; then
    fail "El ejecutable ./build/main no existe."
    exit 1
fi

echo "--- Verificando Memory Leaks ---"
# Ejecutar valgrind. Suprimimos el output estandar y capturamos stderr
LEAK_REPORT=$(valgrind --error-exitcode=1 --leak-check=full ./build/main 2>&1 >/dev/null)

if [ $? -eq 0 ] && echo "$LEAK_REPORT" | grep -q "0 bytes in 0 blocks"; then
    pass "No se detectaron fugas de memoria según Valgrind"
else
    fail "Valgrind detectó fugas de memoria o errores. Revisa tu código."
    echo "$LEAK_REPORT" | grep -i "definitely lost:"
    echo "$LEAK_REPORT" | grep -i "invalid"
fi

echo "Resultado test Valgrind: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ] || exit 1
