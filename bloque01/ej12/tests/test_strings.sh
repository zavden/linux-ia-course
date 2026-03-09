#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 1.2 (Strings Seguros) ==="

cd src/

# Compilar uniendo los dos archivos c. Activar protector de stack para 
# probar que nuestra función ES la que los defiende limpiamente sin crashes del compilador.
gcc -Wall -Wextra -std=c17 -fstack-protector-all main.c safe_strings.c -o string_test || exit 1

OUT=$(./string_test 2>&1)
EXIT_CODE=$?

if [ $EXIT_CODE -ne 0 ]; then
    echo "❌ El código se rompió (segfault/stack smashing). Las funciones no son seguras."
    exit 1
fi

if echo "$OUT" | grep -q "String ma"; then
    echo "✅ safe_strcpy previno el desbordamiento truncando a 9 + NULL (String ma)"
else
    echo "❌ safe_strcpy no truncó en el punto esperado"
    echo "$OUT"
    exit 1
fi

if echo "$OUT" | grep -q "Aviso de truncamiento"; then
    echo "✅ El warning manual de truncamiento funcionó analizando el retorno"
else
    echo "❌ No se detectó el truncamiento retornado de safe_strcpy"
    exit 1
fi

if echo "$OUT" | grep -E "Hola Mundo cru"; then
    echo "✅ safe_strcat concatenó salvaguardando memoria."
else
    echo "❌ Falló safe_strcat"
    exit 1
fi

rm -f string_test
echo "✅ Todos los tests pasados con buffers 100% seguros y limpios!"
