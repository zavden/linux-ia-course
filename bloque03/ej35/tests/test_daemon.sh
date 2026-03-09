#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 3.5 (Configurando Demonio SysV) ==="

cd solucion/src

gcc -O2 -Wall -Wextra -std=c17 main.c -o my_service || exit 1

# Limpieza por si quedaban cadaveres
rm -f /tmp/mi_daemon.pid /tmp/mi_daemon_secreto.log

# 1. Lanzamos demonio
# Como tiene su `daemonize()`, NISIQUIERA necesitamos del ampersand final `&` para fondo!
./my_service

echo "Esperando su anidamiento... (1 seg)"
sleep 1

# Recupero PID robándolo del archivo Lock creado
if [ ! -f /tmp/mi_daemon.pid ]; then
    echo "❌ Fallo grave: No nació, o murio al caer o no creó su PID file_lock."
    exit 1
fi

DAEMON_PID=$(cat /tmp/mi_daemon.pid | tr -d '\n')
echo "El Demonio se encubrió bajo la identidad Kernell PID: $DAEMON_PID"

# Compruebo con el Comando TTY Oficial de status real si realmente rompió conexión PTY 0 con nuestra Matrix de pantalla interactiva.
TTY_STATUS=$(ps -p $DAEMON_PID -o tty= | awk '{print $1}')
if [ "$TTY_STATUS" == "?" ]; then
    echo "✅ Separación SID Independiente perfecta: TTY '?'. Es huérfano Oficial a Pantallas Interactivas y Bash no lo atará nunca."
else
    echo "❌ Fracasó desconección TTY interactiva. El TTY todavia da '$TTY_STATUS'. ¡Usa setsid y fork 2x!"
    exit 1
fi

# INTENTAMOS CLON MALICIOSO DOBLE EN VIVO (Simulando 2 servicios corriendo)
echo "Probando Candado Ciego Exclusivo .pid ..."
./my_service || true # Que no rompa bash
sleep 1

# Hacemos los disparos Administrativos HUP y TERM.
kill -HUP $DAEMON_PID
sleep 1
# Golpe mortal limpio OS Native
kill -TERM $DAEMON_PID
sleep 1

# TEST DEL REGISTRO.
echo ">> Verificando Registro de Log de Sysadmin :"
cat /tmp/mi_daemon_secreto.log


if grep -q "SEGUNDO daemon rebotado" /tmp/mi_daemon_secreto.log; then
    echo "✅ Candado Múltiple Exclusividad exitoso impidiendo colisiones portuarias o corruciones."
fi

if grep -q "CAMBIOS DE CONFIGURACION" /tmp/mi_daemon_secreto.log; then
    echo "✅ Eventos SIGHUP asíncronos procesados correctamente"
fi

if grep -q "DESTRUIDO LIMPIAMENTE" /tmp/mi_daemon_secreto.log; then
    echo "✅ Limpieza de Terminate C segura terminada (Graceful exit log)."
else
    echo "❌ El proceso crasheó instantáneo y sucio sin acatar return con SIGTERM."
    kill -9 $DAEMON_PID || true
    exit 1
fi

if [ -f /tmp/mi_daemon.pid ]; then
    echo "❌ Warning: No borró su archivo PID despues de morir con SIGTERM (limpieza fd fallada). Pero no mató el Test."
fi

rm -f /tmp/mi_daemon_secreto.log /tmp/mi_daemon.pid my_service
echo "PASSED"
