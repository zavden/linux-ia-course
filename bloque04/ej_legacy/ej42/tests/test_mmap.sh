#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 4.2 (mmap mutaciones file-based) ==="

cd solucion/src

gcc -O2 -Wall -Wextra -std=c17 main.c -g -o mmap_test || exit 1

# Generamos archivo de victima con string de 12 bytes identificable
TARGET="test_volatil.txt"
echo "B_Originals" > $TARGET

# Corremos la mutacion de memoria directa
OUT=$(./mmap_test $TARGET)
echo "$OUT"

if echo "$OUT" | grep -q "0 Bytes"; then
    echo "❌ Error. Test Detecto mala interpretacion de st_size"
    exit 1
fi

# El TEST MAS IMPORTANTE Y DEFINITIVO. Que la Magia Shared cruzara hacia el disco físico
VALOR_NUEVO=$(head -c 1 $TARGET)

if [ "$VALOR_NUEVO" == "A" ]; then
   echo "✅ MMAP MAP_SHARED Mutó exitosamente el archivo subyacente modificándolo desde las Sombras de RAM al Disco Duro."
else
   echo "❌ Fracaso Total: La letra en el disco sigue siendo B o Basura. Linux no sincronizó msync() la memoria Cache File o el Pointer falló su Mapeo!"
   exit 1
fi

# Test de resiliencia pidiendo archivos nulos "vacíos a mmap" que explotan.
touch vacio.txt
OUT_VACIO=$(./mmap_test vacio.txt 2>&1 || true)
if echo "$OUT_VACIO" | grep -q '0 dimension bytes'; then
    echo "✅ Control resilitente sobre mmap a 0 sizes evitado y logrado!"
else
    echo "❌ Fallo. El mmap trató debilmente de invocar RAM fantasma sobre un unarchivo 0 size y crasheó OS/Kernell."
    exit 1
fi

rm -f $TARGET vacio.txt mmap_test
echo "PASSED"
