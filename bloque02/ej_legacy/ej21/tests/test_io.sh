#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 2.1 (open/read/write) ==="

cd solucion/src

# Compilacion cruda 
gcc -Wall -Wextra -std=c17 main.c -o my_cp || exit 1

# Generamos un archivo de 5 MB de datos aleatorios pesados
dd if=/dev/urandom of=origen.dat bs=1M count=5 2>/dev/null

echo "Copiando binario..."
./my_cp origen.dat destino.dat

if [ ! -f destino.dat ]; then
    echo "❌ El programa no creó el archivo destino"
    exit 1
fi

HASH_ORIGEN=$(md5sum origen.dat | awk '{print $1}')
HASH_DESTINO=$(md5sum destino.dat | awk '{print $1}')

if [ "$HASH_ORIGEN" == "$HASH_DESTINO" ]; then
    echo "✅ El archivo destino tiene el hash MD5 exacto ($HASH_ORIGEN) al origen."
else
    echo "❌ Los hashes divergen. La rutina de I/O corrompió los datos."
    exit 1
fi

# Test de comportamiento de Falla Elegante
echo "Verificando falla elegante en permisos denegados (requiere root para /etc/shadow, asi que de tu usuario fallara)"
set +e
./my_cp /etc/shadow pwned.txt 2> error.log
EXIT_CODE=$?
set -e

if [ $EXIT_CODE -ne 1 ]; then
    echo "❌ Debió salir con Exit Code 1 al denegarse los permisos"
    exit 1
fi

if ! grep -q "origen: Permission denied" error.log; then
    echo "❌ perror no imprimió adecuadamente el error de permisos"
    exit 1
fi
echo "✅ Validacion segura de error de permisos lograda."


rm -f origen.dat destino.dat my_cp error.log pwned.txt
echo "PASSED"
