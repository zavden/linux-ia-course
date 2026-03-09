#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 7.2 (GDB Core_Dump Autopsia) ==="

cd solucion/src

# CRITICAL FLAG -g : Injecta símbolos legibles (Mapeo linea 24 de C -> 0xFF23x asm!). 
# Sin esto, gdb diría "?? () en ?? C" y sería inútil forense.
gcc -Wall -Wextra -std=c17 main.c -g -o crash_test || exit 1

# Activamos a la fuerza la creación de cadaveres (DUMPS) en Unix/Bash para este Test.
ulimit -c unlimited

# Forzamos los Crashes y no dejamos que Bash pare el Script si ocurre el error intencional. (|| true)
echo "[*] Explotando Caso 1: División Mágica"
./crash_test 1 > out1.log 2>&1 || true

echo "[*] Explotando Caso 2: Null Dereference"
./crash_test 2 > out2.log 2>&1 || true

echo "[*] Explotando Caso 3: Asfixia Recurrente"
./crash_test 3 > out3.log 2>&1 || true

# Test: Existen las muertes en Log Bash?
if grep -q "Floating point exception" out1.log; then
    echo "✅ FPU Abort Processor: Atrapada SIGFPE Exitosamente (Muerte aritmetica OS Validada)."
else
    echo "❌ Error, La division intencional no invocó muerte o GCC lo perdonó."
    exit 1
fi

if grep -q "Segmentation fault" out2.log; then
     echo "✅ MMU Abort: Atrapada Violación Mapeo PTR Exito (SIGSEGv Validado C)."
fi


# Magia Especial: Tragar el análisis si Linux escupio su archivo "core" fisico en este mismo directorio.
# OJO: Los sistemas modernos como Ubuntu desvían los "core" nativos generados al systemsystem `coredumpctl`.
# Para que este test genérico funcione en todos los Linux, comprobaremos GDB "Al Vuelo en Memoria" pasando el CLI Run 
# interactivo automatico sin usar post-fix! 

echo "[*] GDB BATCH TEST: Extrayendo Linea de Muerte Automática del Forense OS!"
# '-ex' instruye a gdb correr comandos solos: Iniciar(ru) , ver el stack al reves(bt), y salirse(q).
GDB_OUTPUT=$(gdb -batch -ex "run 2" -ex "bt" ./crash_test 2>&1 || true)
echo "$GDB_OUTPUT" | grep -A 3 "matar_la_memoria" || true

if echo "$GDB_OUTPUT" | grep -q 'matar_la_memoria'; then
    echo "✅ GDB Autopsia Mágica Triunfante. El forense GNU ubicó exitosamente que el Frame 0 y asesino origen fué la función C \`matar_la_memoria()\` a pesar de haber colapsado."
else
    echo "❌ Fallo del Debugger GNU GDB trace."
    exit 1
fi

rm -f crash_test out*.log core.* 
echo "PASSED"
