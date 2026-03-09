#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 5.5 (Semáforos API Limiter) ==="

cd solucion/src
gcc -O2 -Wall -Wextra -std=c17 main.c -pthread -o sem_test || exit 1

OUT=$(./sem_test)
echo "$OUT"

if echo "$OUT" | grep -q "ATENDIDOS SATISFACTORIAMENTED"; then
    echo "✅ El Loop Batch del Limiter procesó pacientemente todas mis peticiones del Test Script y unió Pthreads correctamente Limpio."
else
    echo "❌ Error De Semáforo Asíncrono! Congelados C Threads muertos."
    exit 1
fi

if echo "$OUT" | grep -q "\-1"; then
    echo "❌ Peligro: Sem_GetValue arrojó pase < 0! Rompiste la matemática y dejaste pasar a más gente limitrofe de OS!"
    exit 1
else 
     echo "✅ Sem_GetValue validó que nunca se exedió del conteo estricto Matemático Posix."
fi

# Cuento en Linux cuantas veces mi String "PROCESANDO PESADO" saltó a Pantalla.
# Si fueron 10 mis ataques simultáneos programados al Servidor, mi greep wc l deberia asomar 10!.
# O sea, que ningun hilo se perdió del cond wait.
C_LINEAS=$(echo "$OUT" | grep "PROCESANDO PESA" -c || true)
if [ "$C_LINEAS" == "10" ]; then
   echo "✅ 10 de 10 peticiones atendidas. Rate limitted Perfecto."
else
   echo "❌ C_LINEAS de procesos pesados arrojó $C_LINEAS! Algún hilo C murió de hambre robándole su turno o fallaron al arrancar."
   exit 1
fi


rm -f sem_test
echo "PASSED"
