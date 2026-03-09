#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 2.2 (stdio vs syscalls) ==="

cd solucion/src

# Compilando
gcc -O2 -Wall -Wextra -std=c17 main.c -lrt -o io_bench || {
    gcc -O2 -Wall -Wextra -std=c17 main.c -o io_bench || exit 1
}

# Ejecutamos. Va a demorar.
OUT=$(./io_bench)

echo "$OUT"

if echo "$OUT" | grep -qi "CONCLUSION: fgetc / stdio.h fue.*VECES MAS RAPIDO"; then
    echo "✅ El benchmark expuso exitosamente el coste de las Syscalls."
else
    echo "❌ No se encontró la diferencia de tiempos esperada en el output."
    exit 1
fi

rm -f io_bench
echo "PASSED"
