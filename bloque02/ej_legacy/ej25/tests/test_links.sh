#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 2.5 (Enlaces y Inodos) ==="

cd solucion/src

gcc -O2 -Wall -Wextra -std=c17 main.c -o links_test || exit 1

OUT=$(./links_test base_test.txt hard_test.txt sym_test.sym)

echo "$OUT"

if echo "$OUT" | grep -q "Datos Rescatados"; then
    echo "✅ El hard link protegió exitosamente los datos tras el borrado del original."
else
    echo "❌ Fallo protegiendo datos con el hard link."
    exit 1
fi

if echo "$OUT" | grep -q "No such file"; then
    echo "✅ El intento de leer un Symlink roto devolvió No Such File."
else
    echo "❌ El Symlink no se reportó como roto tras el unlink del archivo base."
    exit 1
fi

rm -f ./*test* links_test
echo "PASSED"
