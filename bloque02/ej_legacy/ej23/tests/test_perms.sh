#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 2.3 (Permisos y umask) ==="

cd solucion/src

# Compilando
gcc -O2 -Wall -Wextra -std=c17 main.c -o perms || exit 1

# Aseguramos un umask estricto en bash para probar que C lo puentea
umask 0077

echo "Creando archivo con 0644 deseados..."
OUT=$(./perms 0644_test.txt 0644)

# Validando permiso real ignorando la máscara 077.
REAL_PERMS=$(stat -c '%a' 0644_test.txt)

if [ "$REAL_PERMS" == "644" ]; then
    echo "✅ El programa aplicó 0644 ignorando el umask hiper-restrictivo de Bash."
else
    echo "❌ Fallo en open/chmod. Permisos resultaron ser: $REAL_PERMS en lugar de 644."
    exit 1
fi

if echo "$OUT" | grep -q "getfacl"; then
   echo "✅ system(getfacl) verificado y loggeado con éxito."
fi

# Cleanup
rm -f 0644_test.txt perms
echo "PASSED"
