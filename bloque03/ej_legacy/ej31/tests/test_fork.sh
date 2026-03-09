#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 3.1 (fork/waitpid) ==="

cd solucion/src

# Compilando con GCC 
gcc -O2 -Wall -Wextra -std=c17 main.c -o fork_test || exit 1

# Almacenamos el Output del test en bruto
OUT=$(./fork_test)
echo "$OUT"

if echo "$OUT" | grep -q "Código: 10"; then
    echo "✅ Waitpid rescató el valor exit status 10 originado del Hijo[0] aislado con C"
else
    echo "❌ Fallo parseando el WEXITSTATUS(10) esperado para hijo 0"
    exit 1
fi

if echo "$OUT" | grep -q "Código: 12"; then
    echo "✅ Waitpid rescató el valor final esperado 12 originado de los procesos."
else
    echo "❌ No se obtuvo el último exit esperado. Error en bucle while de creación/waitpid."
    exit 1
fi

if echo "$OUT" | grep -q "Intacta"; then
    echo "✅ La variable aislada local de memoria de RAM (DATA) logró comprobar el efecto de forkeo (No intermitió la del padre)."
else
    echo "❌ Error fatal: La variables cruda sí mutó, quizás usaron Thread Pthreads sin querer?"
    exit 1
fi


rm -f fork_test
echo "PASSED"
