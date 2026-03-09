#!/bin/bash
set -euo pipefail

echo "=== Tests Ejercicio C.3 ==="
cd src/

if [ ! -f "Makefile" ]; then
    echo "❌ Makefile no encontrado"
    exit 1
fi

if ! grep -q "%.o.*%.c" Makefile; then
    echo "❌ El Makefile no parece usar la regla de patrón %.o: %.c"
    exit 1
fi

make clean >/dev/null 2>&1 || true
make >/dev/null

if [ ! -x "./calculadora" ]; then
    echo "❌ 'make' falló o no generó calculadora"
    exit 1
fi

OUTPUT=$(./calculadora)
if echo "$OUTPUT" | grep -q "LOG.*Calculadora"; then
    echo "✅ Makefile funciona con múltiples dependencias!"
else
    echo "❌ Output incorrecto"
    exit 1
fi
echo "PASSED"
