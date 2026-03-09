#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 6.1 (Sockers C Y Enrutamiento NetCat Terminal) ==="

cd solucion/src
gcc -O2 -Wall -Wextra -std=c17 main.c -o sock_test || exit 1


echo "Lanzada daemon c servidor puerto pasivo"
./sock_test > server_out.log 2>&1 &
PID=$!
sleep 1 # Arranque boot listen C++

echo "Testeando cliente TCP Intruso 1 (Cerrando de golpe / Crtl C falso / EOF nc)"
echo "Mensaje Secreto" | nc 127.0.0.1 9090

sleep 1

echo "Testeando test 2 Con Desconexion normalizada Exitosa quit TCP"
(echo "hola"; echo "quit"; sleep 0.5) | nc 127.0.0.1 9090

kill -9 $PID || true 
sleep 1

OUT=$(cat server_out.log)
echo "$OUT"

if echo "$OUT" | grep -q 'CONNECTADO.*127'; then
    echo "✅ IP_Exranjera convertida nativamente a string y leída (El Server logueó su LocalHost o Local IP remotamente con ntao() inet)."
else
    echo "❌ Fallo del Bind o Firewall/Docker Kernell no dejando enrutar Antenas puerto 9090 local tcp."
    exit 1
fi

if echo "$OUT" | grep -q 'Mensaje Secreto'; then
    echo "✅ Comunicación Payload transmitida limpia leída en bucles y vaciada del Stream TCP Buffer."
else
    echo "❌ Error en Read Buffer FD. O no entró al while, y rompió la lectura truncándola."
    exit 1
fi


if echo "$OUT" | grep -q 'Desconexión Remota pacífica'; then
    echo "✅ Control String 'quit' implementado exitoso cerrando sesiones C controladas Loop_break TCP!."
else
    echo "❌ No escapó o leyó el comando quit en el read."
    exit 1
fi

rm -f sock_test server_out.log
echo "PASSED"
