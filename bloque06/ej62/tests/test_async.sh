#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 6.2 (Event Loop No Bloqueante OS) ==="

cd solucion/src
gcc -O2 -Wall -Wextra -std=c17 main.c -o nonblock_test || exit 1


echo "Testiando la Consola Virtual Inyectada Asincrona... (Enviamos Quit cruzado en timeout de 3s)"
# Simulamos un delay de 3 segundos para que logre imprimir puntos '...' en terminal y en el 3er segundo mandamos 'quit' a su STDIn via Echo en Pipe C_OS!!
OUT=$( (sleep 3; echo "quit") | ./nonblock_test)
echo "$OUT"


if echo "$OUT" | grep -q '^\.\.\.'; then
    echo "✅ Puntillismo Async Capturado! (Logró evadir Read e iterar de forma asíncrona EAGAIN múltiples sleeps C Segundos Primitivos)."
else
    echo "❌ Peligro: El programa no generó sus puntitos asincrónos de poll.  o bloqueó duro esperando mi texto bash en sleep crudo perdiendo vida."
    exit 1
fi

if echo "$OUT" | grep -q "Capturaste un Teclado Asíncrono puro: 'quit'"; then
    echo "✅ Intercepción 'quit' procesada post-loop y salida validada. El Async Kernell C aceptó Data tardía sin bloqueos muertos."
else
    echo "❌ Error FD Read. No lograste leer o falló Parseo strncmp en buffer C al final del Pipe."
    exit 1
fi

if echo "$OUT" | grep -q "BLOCKING NORMALIZADO"; then
    echo "✅ Mago Superior! Restauraste la terminal Bash Madre a O_BLOCK evitando crashear tu sistema operativo o consola ajena C!"
else 
    echo "❌ OJO! TE OLVIDASTE EL CLEAN UP. Restaurar el FD de fabrica Kernell Bash al final o morirás en Bash Scripts reales con FDs sucios!"
    exit 1
fi

rm -f nonblock_test
echo "PASSED"
