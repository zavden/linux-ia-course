#!/bin/bash
set -euo pipefail

make -s clean
make -s all

cat <<'IN' > entrada.txt
echo hola_mini
exit
IN

./build/main < entrada.txt > salida.txt

grep -q "hola_mini" salida.txt

rm -f entrada.txt salida.txt
echo "E10 OK"
