#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 1.5 (Benchmark Aritmética vs SIMD) ==="

cd src/

# Compilando con máxima optimización y chequeando soporte de timers
gcc -O3 -Wall -Wextra -std=c17 main.c -lrt -o bench_test || {
    # Algunas veces -lrt no es necesario en distros modernas, si falla lo intentamos sin él
    gcc -O3 -Wall -Wextra -std=c17 main.c -o bench_test || exit 1
}

# Ejecutamos
OUT=$(./bench_test)

echo "$OUT"

if echo "$OUT" | grep -qi "CONCLUSION: memcpy nativo es.*veces mas rapido"; then
    echo "✅ El benchmark demostró que no debes reinventar memcpy()."
else
    echo "❌ No se encontró la conclusión del benchmark."
    exit 1
fi

rm -f bench_test
echo "PASSED"
