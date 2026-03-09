#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 5.2 (Data Races Y Mutex Atómicos) ==="

cd solucion/src
gcc -O2 -Wall -Wextra -std=c17 main.c -pthread -o mutex_test || exit 1

OUT=$(./mutex_test)
echo "$OUT"

# Ojo: La anarquía puede A VECES rarísimamente salir bien si tu Kernel es una locura y aisló milagrosamente el scheduling, 
# pero le pedimos 10 Millones... es virtualmente estocástica-mente imposible que no colisionen al menos 1 sola vez en 10 Millones.
if echo "$OUT" | grep -q "FATALIDAD"; then
    echo "✅ Race Condition Inundador provocado exitosamente. Tu memoria corrupta falló el conteo de 10Millones como esperaba el C_Test."
else
    echo "❌ Peligro: El Test Maligno no falló! Increíble magia de tu CPU o pusiste bloqueadores sin querer."
    exit 1
fi

if echo "$OUT" | grep -q "PERFECCIÓN Thread-Safe"; then
    echo "✅ Lock de POSIX (pthread_mutex) resuelto exitoso. El saldo Bancario arrojó exactamente los 10,000,000 esperados."
else
    echo "❌ Error Crítico: Los MUTEX fallaron miserablemente salvando C o no enviaste la Lock Variable por refernecia global."
    exit 1
fi

rm -f mutex_test
echo "PASSED"
