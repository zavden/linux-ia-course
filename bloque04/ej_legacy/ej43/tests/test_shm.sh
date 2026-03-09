#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 4.3 (Telepatía Dual POSIX IPC) ==="

cd solucion/src

# Ojo: Para POSIX shm en algunos OS antiguos requiere ligarlo formalmente linkeando con biblioteca Real-time de Kernell 
# de libc: "-lrt". Modificamos para ser cross-patible seguro con tu C17 formal gcc .
gcc -O2 -Wall -Wextra -std=c17 main.c -lrt -o shared_mem || exit 1

OUT=$(./shared_mem)
echo "$OUT"

if echo "$OUT" | grep -q "Secreto Nacional De Inteligencia IPC"; then
    echo "✅ Telepatía exitosamente comprobada (El String sembrado sobrevivió intacto y el Hijo lo pudo escannear desde la neblina RAM)."
else
    echo "❌ Fallo del volcado Padre -> Hijo. Posible colapso en el strcpy a mmap_shared ó Desfasajes asíncronos en los CPU cycles (El sleep falló)."
    exit 1
fi

if echo "$OUT" | grep -q "Reporte Final Entregado Papi: Mision C cumplida"; then
    echo "✅ El Hijo logró sobreescribir mutante los bytes cruzados del pointer devolviéndolos al padre que los Imprimió Exitosamente en Stdio"
else
    echo "❌ Ping-Pong Bidireccional fallido C. (Padre leyó pura basura o nulls en su Array luego del wait)."
    exit 1
fi

if ls /dev/shm | grep -q "canal_telepatico_43"; then
    echo "❌ ¡ERROR CRITICO DE OOM KERNEL SYSTEM! EL PROGAMA LEAKEO ETERNAMENTE EL MONTAJE VIRTUAL (No borró con Shm-unlink!)."
    rm -f /dev/shm/canal_telepatico_43 || true # Borralo manual o destruira la PC en reinicios.
    exit 1
fi

echo "✅ Aseo impecable. `shm_unlink` erradicó exitosamente todo de la red virtual de linux root"


rm -f shared_mem
echo "PASSED"
