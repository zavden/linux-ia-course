#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 5.4 (Lock RW Pthreads Tests) ==="

cd solucion/src
gcc -O2 -Wall -Wextra -std=c17 main.c -pthread -o rw_test || exit 1

OUT=$(./rw_test)
echo "$OUT"

if echo "$OUT" | grep -q "EL MUNDO SE DETIENE"; then
    echo "✅ Lock Táctico de Writer (wrlock) emitido y capturado, silenciando y frenando a las lecturas C++ con Éxito."
else
    echo "❌ Error de Escitura global wrlock, hilo evadió el control o el Main principal lo asfixió matando todos tempranamente."
    exit 1
fi

if echo "$OUT" | grep -q "Total Acumulado Final: 3000"; then
    echo "✅ Matemática Mutante de Escritores Consolidada y Thread-Safe (2 Escritores x 3 Veces * 500 = 3000). 0 Crashes."
else
    echo "❌ El Write Lock falló una Suma Mutante en la iteracion forjada desalineando la Matemática final DB!!."
    exit 1
fi


rm -f rw_test
echo "PASSED"
