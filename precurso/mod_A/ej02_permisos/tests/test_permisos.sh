#!/bin/bash
set -e
echo "=== Tests Ejercicio A.2 (Permisos) ==="

P_SECRETO=$(stat -c "%a" src/secreto.sh 2>/dev/null || echo "000")
P_CONFIG=$(stat -c "%a" src/config.cfg 2>/dev/null || echo "000")

if [ "$P_SECRETO" == "700" ]; then
    echo "✅ secreto.sh ok"
else
    echo "❌ secreto.sh falló (esperado 700, actual $P_SECRETO)"
    exit 1
fi

if [ "$P_CONFIG" == "400" ]; then
    echo "✅ config.cfg ok"
else
    echo "❌ config.cfg falló (esperado 400, actual $P_CONFIG)"
    exit 1
fi

echo "PASSED"
