#!/bin/bash
set -euo pipefail
echo "=== Tests Proyecto 06 (MiniCache Epoll DB TCP Asíncrono) ==="

cd solucion/

make clean >/dev/null
make >/dev/null

echo "Lanzando minicache DB de Background C Epoll..."
./minicache > db_out.log 2>&1 &
SERVER_PID=$!
sleep 1 # Wakeup Binds

echo "Testeando cliente TCP Intruso NC Protocoling... (SET Y GET)"

# Enviaremos secuencias de string y guardaremos la respuesta de NC (NetCat es el curl magico TCP raw) en Out Terminal C
VEREDICTS_RAW_NC=$( (echo "SET Linux Kernell" ; sleep 0.2 ; echo "GET Linux") | nc 127.0.0.1 9595 )

kill -9 $SERVER_PID || true
sleep 0.5 

echo "$VEREDICTS_RAW_NC"

if echo "$VEREDICTS_RAW_NC" | grep -q 'OK'; then
    echo "✅ Comando [SET] TCP Interpretado, y Cache Guardado Mágicamente (OS Server Validó protocolo OK!)."
else
    echo "❌ Error De Socket SET C Array! La string TCP Falló el sscanf enrutamiento."
    exit 1
fi

if echo "$VEREDICTS_RAW_NC" | grep -q 'VALUE.*Kernell'; then
    echo "✅ Asincronismo I/O Triunfante y Protocolo GET validado. El Sistema recuperó Atómicamente el 'Kernell' DB guardado RAM de Array."
else
    echo "❌ Error de Cache Retrieval. El Server C++ falló encontrando el String (Match o Memory Leak Struct OS)."
    exit 1
fi


rm -f minicache db_out.log
echo "PASSED"
