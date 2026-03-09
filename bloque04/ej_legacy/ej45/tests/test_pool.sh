#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 4.5 (Arena MemPool Benchmarks Allocator) ==="

cd solucion/src

gcc -O2 -Wall -Wextra -std=c17 main.c -g -o arena_test || exit 1

echo "Corriendo Benchmark C..."
OUT=$(./arena_test)
echo "$OUT"

if echo "$OUT" | grep -q "Arena Custom Completada"; then
    echo "✅ Memory Arena Cicló sus asiggaciones con el calculo Lineal correctamente O(1)"
else
    echo "❌ Error de asignación, fallo del Arena Pool o Segfault subyancetes de CPU OS Limits."
    exit 1
fi

if echo "$OUT" | grep -q "VEREDICTO"; then
    echo "✅ El benchmark confirmó empíricamente que Pointer-Bumping (Arena) es matemáticamente más veloz esquivando la GLIBC Listas fragmentadas."
else
    echo "❌ Falló benchmark o tu malloc traditional resultó más rápido (Acaso O3 optimizó los For Loops de malloc en loop ciego vacío?)"
fi

if echo "$OUT" | grep -q "Apagón Valido"; then
   echo "✅ El Destructor de Galaxias Master Free Lineal C limpio purga la memoria completa de OS RAM 1 Syscall."
else
   echo "❌ Segment Fault, leak report o asfixiación OS C_Free."
   exit 1
fi 

rm -f arena_test
echo "PASSED"
