#!/bin/bash
set -euo pipefail

PASS=0
FAIL=0

pass() { echo "  ✅ PASS: $1"; ((PASS++)); }
fail() { echo "  ❌ FAIL: $1"; ((FAIL++)); }

echo "=== Tests Ejercicio 0.3 (Docker Compose) ==="

# 1. Comprobar que existe el docker-compose.yml
if [ -f "docker-compose.yml" ]; then
    pass "docker-compose.yml existe"
else
    fail "docker-compose.yml no existe"
    exit 1
fi

# 2. Levantar los servicios y capturar log
echo "Reconstruyendo containers... esto puede tardar unos segundos"
OUTPUT=$(docker compose up --build 2>&1 || true)

if echo "$OUTPUT" | grep -q "Multi-Distro Test"; then
    pass "El codigo se compilo y ejecuto correctamente"
else
    fail "No se encontro el output esperado del programa en los logs"
fi

if echo "$OUTPUT" | grep -qi "fedora.*Compilado con GCC"; then
    pass "El output de fedora es visible en Compose"
else
    fail "Falta el log del contenedor de fedora"
fi

if echo "$OUTPUT" | grep -qi "debian.*Compilado con GCC"; then
    pass "El output de debian es visible en Compose"
else
    fail "Falta el log del contenedor de debian"
fi

echo "Resultado test completo: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ] || exit 1
