#!/bin/bash
set -euo pipefail

PASS=0
FAIL=0

pass() { echo "  ✅ PASS: $1"; ((PASS++)); }
fail() { echo "  ❌ FAIL: $1"; ((FAIL++)); }

echo "=== Tests Ejercicio 0.2 (Makefile) ==="

# 1. Comprobar que existe build/main
if [ -x "build/main" ]; then
    pass "build/main existe y es ejecutable"
else
    fail "build/main no existe (asegúrate de hacer 'make all' primero)"
    exit 1
fi

# 2. Comprobar que los .o y .d existen
OBJS=$(find build -name "*.o" | wc -l)
DEPS=$(find build -name "*.d" | wc -l)

if [ "$OBJS" -eq 3 ]; then
    pass "Se generaron 3 archivos .o en build/"
else
    fail "Se esperaban 3 archivos .o, se encontraron $OBJS"
fi

if [ "$DEPS" -ge 3 ]; then
    pass "Se generaron archivos .d en build/"
else
    fail "No se encontraron archivos .d (falta -MMD -MP)"
fi

# 3. Comprobar output
OUTPUT=$(./build/main)
if echo "$OUTPUT" | grep -q "Información de Build" && echo "$OUTPUT" | grep -q "Make Learner"; then
    pass "El programa funciona y llama a ambos modulos"
else
    fail "Output incorrecto: $OUTPUT"
fi

echo "Resultado test completo: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ] || exit 1
