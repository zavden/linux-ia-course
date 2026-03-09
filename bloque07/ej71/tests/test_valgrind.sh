#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 7.1 (Valgrind Bug Vulnerability Sweeps) ==="

cd solucion/src
gcc -O2 -Wall -Wextra -std=c17 main.c -g -o bug_test || exit 1

echo "[*] INYECTANDO CASO 1: LEAK (Malloc Sin Free)"
# --error-exitcode: Si valgrind detecta basura, no arrojará 0 como siempre, arrojará un 44 crash asincrono de mi bash_test!
OUT_1=$(valgrind --leak-check=full --show-leak-kinds=all ./bug_test 1 2>&1 || true)
echo "$OUT_1" | grep -A 2 "definitely lost:" || true

if echo "$OUT_1" | grep -q 'definitely lost:.*bytes in 1 blocks'; then
    echo "✅ Fuga Detectada Precisamente (Valgrind Atrapó el Bloque Huérfano de OS sin Free)."
else
    echo "❌ Fallo del Engine o Leak muy pequeño OS C_Mem"
    exit 1
fi
echo " "


echo "[*] INYECTANDO CASO 2: USE-AFTER-FREE (Toque a Pointer Muerto Zombie)"
OUT_2=$(valgrind --leak-check=full ./bug_test 2 2>&1 || true)
echo "$OUT_2" | grep -A 2 "Invalid" || true

if echo "$OUT_2" | grep -q 'Invalid read of size 1'; then
    echo "✅ Toque Muerto Detectado (Detectó que intentaste Leer el printf de %s de Puntero Invalido Liberado!)"
else
    echo "❌ Error De Lectura, Valgrind no avisó de la corrupcion Use_A_Free!"
    exit 1
fi
echo " "


echo "[*] INYECTANDO CASO 3: DESBORDAMIENTO ARRAY (Buffer Overflow Limit)"
OUT_3=$(valgrind --leak-check=full ./bug_test 3 2>&1 || true)
echo "$OUT_3" | grep -A 2 "Invalid write" || true

if echo "$OUT_3" | grep -q 'Invalid write of size 4'; then
    # Size 4 Porque es Array de type "INT" puro, int = 4 bytes en Ram!!. (Asombrosa Exactitud de VM Valgrind). 
    echo "✅ Desbordamiento Cazado! (Trataste de PISAR matemáticamente C Array ID[13] fuera de los 10 Limites y Valgrind interrumpió la escritura 4 bytes!)."
else
    echo "❌ Error Buffering C Overflow Failed! Valgrindo Omitió corrupcion."
    exit 1
fi

rm -f bug_test
echo "PASSED"
