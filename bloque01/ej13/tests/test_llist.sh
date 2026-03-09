#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 1.3 (Listas y Memoria Dinámica) ==="

cd src/

# Compilar uniendo los dos archivos
gcc -g -Wall -Wextra -std=c17 main.c llist.c -o l_test || exit 1

OUT=$(./l_test)
if echo "$OUT" | grep -q 'Salió: 200'; then
    echo "✅ Lista inserta y extrae correctamente"
else
    echo "❌ Lógica fallida en inserción/extracción"
    exit 1
fi

# El test real es asegurar que llist fue limpia. Usaremos Valgrind.
if command -v valgrind &> /dev/null; then
    echo "Verificando memory leaks locales..."
    if valgrind --leak-check=full --error-exitcode=1 ./l_test >/dev/null 2>&1; then
        echo "✅ Valgrind no reportó MEMORY LEAKS. ¡Tu gestión de memoria vuela limpio!"
    else
        echo "❌ Valgrind detectó MEMORY LEAKS o invalid reads en tus listas enlazadas. Revisa llist.c o pop/destroy."
        # No matamos el test solo por esto temporalmente si al usuario le faltan libs de debugin
        exit 1
    fi
else
    echo "⚠️ Valgrind no está instalado en este host para probar a full, pero la funcionalidad lógica funciona."
fi

rm -f l_test
echo "PASSED"
