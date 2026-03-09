#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 3.4 (Signals API Control) ==="

cd solucion/src

gcc -O2 -Wall -Wextra -std=c17 main.c -o sig_test || exit 1

# Tenemos que captar la salida asincrona del test, pero en bash no se atrapa facil procesos demonios 
# ya que si le hacemos > al /out correran en fondo las peticiones. Ppondremos output > out.log
./sig_test > test_out.log 2>&1 &
APP_PID=$!

echo "Lanzando demonio temporal con PID Fantasma $APP_PID y aguardando que despierte sus motores 3 sec..."
sleep 4 # Deberia ser suficiente tiempo al C de saltar sus defensas del `sleep(3)` sigprocmask en arranque

echoc "Enviandole Balazo 1 de Crtl+C... (SIGINT/2)"
kill -INT $APP_PID
sleep 1
echoc "Enviandole Balazo Señal Administrativa de Background... (SIGUSR1)"
kill -USR1 $APP_PID
sleep 1
echoc "Enviandole Muerte Asistida Formal... (SIGTERM/15)"
kill -TERM $APP_PID
sleep 1

echo "Verificando el Logs recuperado final:"
cat test_out.log

if grep -q "Sobrevivi al SIGINT" test_out.log; then
    echo "✅ Escudo Bloqueado Inmortal y Crtl+C desviados a voluntad exitosamente (Volatiles)."
else
    echo "❌ Error SIGINT, escudo no levanto en C o mato tu main prematuramente (crédito faltante de handlers?)."
    # Kill the app unconditionally incase of loop failure before exiting.
    kill -9 $APP_PID || true
    rm -f test_out.log sig_test
    exit 1
fi

if grep -q "Señal SIGUSR1 Privada" test_out.log; then
    echo "✅ Disparador IPC asíncrono SIGUSR1 Funciona."
else
    kill -9 $APP_PID || true
    echo "❌ Error SIGUSR1 no emitió mensaje y falló"
    exit 1
fi

if grep -q "Limpieza Exitosa" test_out.log; then
    echo "✅ Shutdown Exitoso pacífico saliendo ETERNAL WHILE a causa de Muerte dictada pacífica (TERM -> Flag 0)."
else
    kill -9 $APP_PID || true
    echo "❌ Error fatal: Murió a traición y sin honor (Nunca finalizó su return Exit Success de su int MAIN ni liberó BDs)."
    exit 1
fi

rm -f test_out.log sig_test
echo "PASSED"
