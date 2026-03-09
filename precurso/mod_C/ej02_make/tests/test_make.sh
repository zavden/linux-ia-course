#!/bin/bash
set -euo pipefail

echo "=== Tests Ejercicio C.2 (Tu Primer Makefile) ==="
cd src/

if [ ! -f "Makefile" ]; then
    echo "❌ Makefile no encontrado en src/Makefile"
    exit 1
fi

make clean >/dev/null 2>&1 || true

make calculadora
if [ ! -x "./calculadora" ]; then
    echo "❌ make calculadora falló o no generó ./calculadora"
    exit 1
fi

if [ ! -f "main.o" ] || [ ! -f "math.o" ]; then
    echo "❌ Faltan los archivos .o. Asegúrate de compilar en dos fases."
    exit 1
fi

make clean
if [ -f "main.o" ] || [ -f "calculadora" ]; then
    echo "❌ make clean no borró los archivos generados"
    exit 1
fi

echo "✅ Makefile funciona correctamente!"
echo "PASSED"
