#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 4.1 (DynArray + Valgrind Memory Integrity) ==="

cd solucion/src

gcc -O2 -Wall -Wextra -std=c17 main.c -g -o dynarray || exit 1

# En este modulo Valgrind no es optativo, Valgrind es un Juez que verifica en camara lenta
# y condena todo leak al fracaso formal
echo "Corriendo simulación física de Máquina Virtual Memcheck (Valgrind)"

OUT=$(valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=100 ./dynarray 2>&1)
echo "$OUT"

# Exit code 100 lo pusimos nosotros si habia algun leak reportado o memory violations
if [ $? -eq 100 ]; then
    echo "❌ Valgrind detectó fugas huérfanas o Violaciones asíncronas de Memoria."
    exit 1
fi

if echo "$OUT" | grep -q "0 errors from 0 contexts"; then
    echo "✅ Análisis Memcheck de Heap Limpio perfecto y liberado de fallos (0 Incorrections)."
else
    echo "❌ Existe un warning flotante no fatal dentro de Valgrind. Revisa los prints."
    exit 1
fi

if echo "$OUT" | grep -q "Re-Alocación.*16"; then
     echo "✅ Las transiciones de Realloc Amortizantes 2x saltaron exitosamente y en su debida logaritmia de tiempo."
else
     echo "❌ Error de progresión aritmética de Capacity dentro del Realloc logic."
     exit 1
fi

rm -f dynarray
echo "PASSED"
