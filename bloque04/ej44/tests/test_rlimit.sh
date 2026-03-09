#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 4.4 (Límites Ulimit Setrlimit Nativos) ==="

cd solucion/src

gcc -O2 -Wall -Wextra -std=c17 main.c -g -o rlimit_test || exit 1

echo "Lanzando martir constringent... 5mb de RAM Max vs Bucle de 20 MegaBites"
OUT=$(./rlimit_test)
echo "$OUT"

if echo "$OUT" | grep -q 'Malloc denegado'; then
    echo "✅ El Kernel Linux cumplió el Contrato Ulimit: Cortó secamente la Glibc impidiendo el Loop y Salvó a la PC."
else
    echo "❌ Fracasó Setrlimit: Rompió o sobrepasó su límite de RAM consumiendo sin parar O crasheó duro por Segfault antes de ver el Null Check C."
    exit 1
fi

if echo "$OUT" | grep -q 'Tajada De Basura MB Asignada Ok: 4'; then
    echo "✅ Efectivamente llenó y ocupó páginas de RAM por lo menos unos 4 tramos de 1 MB seguidos (el lazy pageload conmemset operó el test físico vivo real OS)."
else
    echo "❌ Se detuvo en la primera vuelta o el Kernel ni siquiera cargó los Memset y Lazy Pages C."
    exit 1
fi

if echo "$OUT" | grep -q 'Supo rendirse'; then
    echo "✅ Ejecución de cierre pacífico de Main Loop confirmada."
else
    echo "❌ Crasheó sin llegar al return EXIT_SUCCESS. Control de Free abortado u OOM killed lo cazó de las sombras"
    exit 1
fi

rm -f rlimit_test
echo "PASSED"
