#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 1.1 (Argumentos) ==="

# Compilar rápido para el test
gcc -Wall -Wextra -std=c17 src/main.c -o app || exit 1

# Test 1: Ayuda (-h y --help)
./app -h | grep -q "Opciones:" || { echo "❌ Fallo -h"; exit 1; }
./app --help | grep -q "Opciones:" || { echo "❌ Fallo --help"; exit 1; }

# Test 2: Cortas sin args
OUT=$(./app)
echo "$OUT" | grep -q "Verbose: OFF" || { echo "❌ Fallo default verbose"; exit 1; }
echo "$OUT" | grep -q "Output: salida.txt" || { echo "❌ Fallo default output"; exit 1; }
echo "$OUT" | grep -q "Number: 0" || { echo "❌ Fallo default number"; exit 1; }

# Test 3: Cortas combinadas
OUT=$(./app -v -o test.log -n 50)
echo "$OUT" | grep -q "Verbose: ON" || { echo "❌ Fallo -v"; exit 1; }
echo "$OUT" | grep -q "Output: test.log" || { echo "❌ Fallo -o"; exit 1; }
echo "$OUT" | grep -q "Number: 50" || { echo "❌ Fallo -n"; exit 1; }

# Test 4: Opciones LARGAS
OUT=$(./app --verbose --output=largo.txt --number=999)
echo "$OUT" | grep -q "Verbose: ON" || { echo "❌ Fallo --verbose"; exit 1; }
echo "$OUT" | grep -q "Output: largo.txt" || { echo "❌ Fallo --output"; exit 1; }
echo "$OUT" | grep -q "Number: 999" || { echo "❌ Fallo --number"; exit 1; }

# Test 5: Fallo con opción incorrecta (esperando Exit 1)
if ./app -x 2>/dev/null; then
    echo "❌ Debe fallar con -x"
    exit 1
fi

rm -f app
echo "✅ Todos los tests pasados!"
