#!/bin/bash
set -euo pipefail

echo "=== Tests Ejercicio A.3 (Redirecciones) ==="

cd src/

if [ ! -x "./filtrar.sh" ]; then
    echo "❌ src/filtrar.sh no existe o no tiene permisos de ejecución (+x)"
    exit 1
fi

rm -f errores.log
./filtrar.sh

if [ ! -f "errores.log" ]; then
    echo "❌ errores.log no fue creado"
    exit 1
fi

COUNT=$(wc -l < errores.log | tr -d ' ')
if [ "$COUNT" -eq 3 ]; then
    echo "✅ errores.log contiene 3 líneas"
else
    echo "❌ errores.log contiene $COUNT líneas, se esperaban 3. Filtra solo ERROR."
    exit 1
fi

if grep -q "INFO" errores.log; then
    echo "❌ errores.log contiene líneas de INFO. Deben ser solo ERROR."
    exit 1
fi

echo "✅ Filtro correcto!"
echo "PASSED"
