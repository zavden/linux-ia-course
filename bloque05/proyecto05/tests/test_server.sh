#!/bin/bash
set -euo pipefail
echo "=== Tests Proyecto 05 (OS MiniServer C Pool MultiThreads) ==="

cd solucion/

make clean >/dev/null
make >/dev/null

echo "Lanzando miniserver Web de Background..."
# Ejecutamos con & para que quede corriendo atrás y bash libre de hacer curl despues!
./miniserver > test_out.log 2>&1 &
SERVER_PID=$!

# Descanso pasivo para calentar el Bind y Listen Kernel Os Sockets
sleep 1

# TEST 1: CURL NORMAL HTTP Request Request Crudo
echo "Lanzando Curl Magico Local a mi Puerto Oculto C 8080..."

# Fail true opcional (si un ping da server malformed http fail, el script de tests crashearía). 
HTTP_RESPONSE=$(curl -s -v http://127.0.0.1:8080 -H "Connection: close" 2>&1 || true)

# Validando Headers puristas
if echo "$HTTP_RESPONSE" | grep -q "HTTP/1.1 200 OK"; then
    echo "✅ Cabecera Exitosísima (El servidor procesó y parsió protocolo TCP Correctamente C 200!)."
else
    echo "❌ Error De Sockets. El servidor devolvió algo malo, abortó de red Tcp o crasheo tu PC OS/Curl falló."
    kill -9 $SERVER_PID
    exit 1
fi

if echo "$HTTP_RESPONSE" | grep -q "CONEXION POSIX EXITOSA"; then
     echo "✅ Payload Web HTML extraído, transmitido y descargado limpiamente del hilo Worker OS"
fi

# Asombro Total de QA y CI: 
echo "Matando mi servidor test formal con Sig Kill..."
kill -9 $SERVER_PID || true
sleep 1 # Tiempo para liberar el Port Socket 8080 en el Host antes de devolver shell C

# Observando Los logs Crudos Generados desde adentro (No de red C)
cat test_out.log

if grep -q "Request Web GET exitoso" test_out.log; then
    echo "✅ El Worker HTTP C se loggeó a la consola madre su éxito C internamente confirmando que el Array Central se distribuyó sano a un hilo Random!"
else
     echo "❌ El manager falló asignando a Queue Arrays el T_Socket enrutado en Accept (Posible desincronización Pthread Mutex OS/Condition C_Queue Fallida)."
     exit 1
fi

rm -f test_out.log miniserver
echo "PASSED"
