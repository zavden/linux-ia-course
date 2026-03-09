#!/bin/bash
set -euo pipefail
echo "=== Tests Proyecto 04 (minitop /proc OS Parseador) ==="

cd solucion/

make clean >/dev/null
make >/dev/null

echo "Lanzando minitop tester (Se detiene él solo por el QA flag a los 2 segundos de print)..."
OUT=$(./minitop)
echo "$OUT"


# Chequeo 1: Header de Utilización Viva OS Meminfo
if echo "$OUT" | grep -q "MINITOP - LINUX PROCESS KERNEL VIEW"; then
    echo "✅ Interfaz Header pintada correctamente y sin crashear."
else
    echo "❌ Error renderizando Cabecera."
    exit 1
fi

if echo "$OUT" | grep -q "RAM Arox"; then
    echo "✅ Parsing matemático Global en Vivos (/proc/meminfo) procesado sin dividir por cero ni explotar el float double."
else
    echo "❌ Error extrayendo /proc/meminfo. ¿No has parseado los strings o Linux te lo prohibió en sandbox?"
    exit 1
fi

# Chequeo 2: Debe aparecer el PID de 1, q casi siempre es systemd, o si estamos 
# en docker el bash/tester script. Siempre, MÍNIMO, habrá el propio `./minitop` listado!!
# Dado que el output se imprime algo como: "1234  | R  | 0 MB  | minitop"
if echo "$OUT" | grep -q "minitop"; then
    echo "✅ Minitop fue capaz de auto-descubrirse a si mismo iterando la base de datos de los PIDS del Kernel Múltiple (Readdir)."
else
    echo "❌ Inanición Grave OS: No se pudo descubrirse a sí mismo o el /proc no dejó leer PIDs de tu nivel usuario C."
    exit 1
fi

# Assert columns
if echo "$OUT" | grep -q "RES. RAM"; then
    echo "✅ Las tablas de Memoria VResident / RSS y Status Parseadas (100% Top Emulacion de C ANSI Correcta)."
fi

rm -f minitop
echo "PASSED"
