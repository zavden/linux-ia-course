#!/bin/bash
set -euo pipefail
echo "=== Tests Proyecto 01 (miniecho & minicat) ==="

cd solucion/

# Compilamos usando el Makefile
make clean >/dev/null
make >/dev/null

# ---- TEST MINIECHO ----
echo ">> Testeando miniecho..."

# 1. Básico
OUT=$(./miniecho hola mundo)
if [ "$OUT" != "hola mundo" ]; then
    echo "❌ miniecho normal falló: '$OUT'"
    exit 1
fi

# 2. Sin salto de linea (-n)
OUT=$(./miniecho -n hola mundo)
if [ "$OUT" != "hola mundo" ]; then
    echo "❌ miniecho -n falló."
    exit 1
fi

# 3. Escapes (-e)
# Deberia imprimir A y luego B en la sig linea, pero como lo capturamos en variable, se mide crudo
OUT=$(./miniecho -e "A\nB")
REAL_ECHO=$(echo -e "A\nB")
if [ "$OUT" != "$REAL_ECHO" ]; then
    echo "❌ miniecho -e falló con los escapes."
    exit 1
fi


# ---- TEST MINICAT ----
echo ">> Testeando minicat..."

echo "Linea 1" > temp_test.txt
echo "Linea 2" >> temp_test.txt

# 1. Lectura de Archivo
OUT=$(./minicat temp_test.txt)
if ! echo "$OUT" | grep -q "Linea 2"; then
    echo "❌ minicat falló en lectura de archivo normal."
    exit 1
fi

# 2. Error simulado en archivo falso
set +e
./minicat archi_falso.txt 2> error.log
EXIT_CODE=$?
set -e
if [ $EXIT_CODE -ne 1 ]; then
    echo "❌ minicat no retornó Exit Failure (1) al faltar el archivo."
    exit 1
fi
if ! grep -q "minicat:" error.log; then
    echo "❌ minicat no imprimió perror con prefijo 'minicat:'"
    exit 1
fi

rm -f temp_test.txt error.log miniecho minicat

echo "✅ Ambos clones de GNU coreutils funcionan como los originales!"
echo "PASSED"
