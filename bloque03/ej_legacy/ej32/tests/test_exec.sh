#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 3.2 (execvp / dup2 / redirección) ==="

cd solucion/src

gcc -O2 -Wall -Wextra -std=c17 main.c -o exec_test || exit 1

# TEST DE CÁRCEL
echo "Prueba 1: Redirección exitosa del binario 'echo' inyectado"
# El array mandado seria {"echo", "A", "B", "destino.txt"} -> "destino.txt" sera quitado por null

./exec_test echo "Infeccion" "123" test_destino.txt >/dev/null

if [ ! -f test_destino.txt ]; then
    echo "❌ Fallo grave: No se generó test_destino.txt."
    exit 1
fi

if grep -q "Infeccion 123" test_destino.txt; then
    echo "✅ El mensaje del 'echo' forkeado fue enjaulado mágicamente dentro del output vía DUP2."
else
    echo "❌ Falló el enlace de la canería dup2() o la posesión no despachó 'echo' de la ruta."
    exit 1
fi

# TEST COMPROBANDO RECHAZO: Comando Inexistente
echo "Prueba 2: Interceptado del Kernel fallando un execvp en vivo."
OUT=$(./exec_test comando_mega_extrano 1 2 basurita.txt 2>&1 || true)
echo "$OUT"

if echo "$OUT" | grep -q "Comando perdido" || echo "$OUT" | grep -qi "fall"; then
    echo "✅ Ejecución de un binario inexistente detectada limpia cruzando la Línea de los Caídos sin crashear al Padre."
else
    echo "❌ No se activó perror debajo de execvp, reencarnó o engañó con Exitcode=0 un error nulo."
    exit 1
fi

rm -f test_destino.txt basurita.txt exec_test
echo "PASSED"
