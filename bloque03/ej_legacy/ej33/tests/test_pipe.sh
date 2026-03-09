#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 3.3 (Inter-Process Pipes) ==="

cd solucion/src

gcc -O2 -Wall -Wextra -std=c17 main.c -o ipc_test || exit 1

OUT=$(./ipc_test)
echo "$OUT"

if echo "$OUT" | grep -q "Ejecutar Plan Alfa"; then
    echo "✅ Padre transmitió correctamente los datos unidimensionalmente hacia Hacia el hijo"
else
    echo "❌ Fallo del read()/write() de bajada en el TRAMO 1."
    exit 1
fi

if echo "$OUT" | grep -q "Comprendido Comandante Padre"; then
    echo "✅ El Hijo recibió la ráfaga, y retornó un acuse de recibo mágico independiente al Padre interceptado"
else
    echo "❌ Ping-Pong Bidireccional fallido: Ocurrió un Deadlock (Congelados para siempre) o hubo un pipe Roto."
    exit 1
fi

rm -f ipc_test
echo "PASSED"
