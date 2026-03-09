#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 5.1 (Pthreads Confluentes MultiCore) ==="

cd solucion/src

# REQUIRED BY LINUX COMPILERS: Linkeador Obligatorio "-pthread" or GCC EXPLOTARA por "undefined reference" a posix apis
gcc -O2 -Wall -Wextra -std=c17 main.c -pthread -o multi_test || exit 1

OUT=$(./multi_test)
echo "$OUT"

if echo "$OUT" | grep -q "Aterrizó vivo.*T#3"; then
    echo "✅ 4 Pthreads independientes creados despachados al Kernel y confirmados por Log Asíncrono."
else
    echo "❌ Fallo grave en pthread_create, faltaron hilos o no loggearion su nacimiento crudo."
    exit 1
fi


if echo "$OUT" | grep -q "WIN"; then
    echo "✅ Verificación de Array mutación comprobada global C. Benchmarks correctos y sincronizados por el Join_LOOP!."
else
    echo "❌ Verificacion Pthread_Join fallida (Crasheaste dejando array huérfana de variables, re-checa uniones de Punteros)."
    exit 1
fi

rm -f multi_test
echo "PASSED"
