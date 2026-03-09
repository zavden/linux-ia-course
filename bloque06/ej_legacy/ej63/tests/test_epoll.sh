#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 6.3 (Epoll Router Master) ==="

cd solucion/src
gcc -O2 -Wall -Wextra -std=c17 main.c -g -o epoll_test || exit 1

echo "Lanzada daemon c servidor puerto pasivo EPOLL 9090"
./epoll_test > epoll_out.log 2>&1 &
PID=$!
sleep 1 

echo "Testeando cliente TCP Intruso 1 C"
echo "Hola epoll" | nc 127.0.0.1 9090

# Simulamos la "Multiconcurrcia 10,000" pero con 3. Abriremos 3 procesos de netcat a la misma putisima vez asincronos en bash &.
echo "Llovizna Simultanea Asincrona C. 3 Bash Backgrounds atacando The Matrix!:"
(sleep 0.2; echo "Infiltrado_1"; sleep 0.2;) | nc 127.0.0.1 9090 &
(sleep 0.2; echo "Infiltrado_2"; sleep 0.2;) | nc 127.0.0.1 9090 &
(sleep 0.2; echo "Infiltrado_3"; sleep 0.2;) | nc 127.0.0.1 9090 &

sleep 2 # Dejamos la tormenta c_os procesarse pura...

kill -9 $PID || true 
sleep 0.5

OUT=$(cat epoll_out.log)
echo "$OUT"


if echo "$OUT" | grep -q 'Conectado y En-Radar'; then
    echo "✅ Accept Masivos logrados y enrutados por matriz principal C al epoll master (EPOLL_CTL_ADD Trabajó)."
else
    echo "❌ Peligro: El evento del papa (Master TCP accept) falló. ¿Rompiste la iteracion EAGAIN Non-Blocking de Accept o no pusiste INADDR C ?"
    exit 1
fi

if echo "$OUT" | grep -q 'Infiltrado_3'; then
    echo "✅ Toda la llovisna simultánea cruzó y el Servidor (Con ¡1 SOLO HILO C!) Fue capaz absorber 4 Peticiones Múltiples C Sin Parpadeos Ni Locks!."
else
    echo "❌ Error De Bloqueo Crudo. Tu Epoll se trabó durmiendo mal en edge-triggered loop (read buf faltó while eaglein!) "
    exit 1
fi

rm -f epoll_test epoll_out.log
echo "PASSED"
