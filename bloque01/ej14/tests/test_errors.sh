#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 1.4 (Errores POSIX) ==="

cd src/

gcc -g -Wall -Wextra -std=c17 main.c -o err_test || exit 1

set +e
OUT=$(./err_test 2>&1)
EXIT_CODE=$?
set -e

if [ $EXIT_CODE -ne 1 ]; then
    echo "❌ El programa debió salir con código 1 (Failure)"
    exit 1
fi

if echo "$OUT" | grep -q "ERROR.*main\.c:"; then
    echo "✅ Macro CHECK detectó el error e imprimió el archivo/línea (main.c)"
else
    echo "❌ No se imprimió el error con formato de Archivo/Línea esperado."
    exit 1
fi

if echo "$OUT" | grep -q "No such file or directory"; then
    echo "✅ El Motivo (perror/errno) fue interpretado correctamente por el OS."
else
    echo "❌ Faltó usar perror() o strerror() para el motivo nativo del Kernel."
    exit 1
fi

if echo "$OUT" | grep -q "cleanup"; then
    echo "✅ Se llegó al unificado cleanup!"
else
    echo "❌ No se ejecutó el bloque cleanup."
    exit 1
fi

# Valide memory leaks temporales
if command -v valgrind &> /dev/null; then
    if valgrind --leak-check=full --error-exitcode=1 ./err_test >/dev/null 2>&1; then
        echo "✅ Valgrind no reportó MEMORY LEAKS. Cleanup destruyó exitosamente el malloc()."
    else
        echo "❌ Valgrind detectó fugas. Revisa tu etiqueta cleanup."
        exit 1
    fi
fi

rm -f err_test
echo "PASSED"
