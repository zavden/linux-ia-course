#!/bin/bash
# Test para Ejercicio 0.1 — Hello Docker
set -euo pipefail

BINARY="./build/main"
PASS=0
FAIL=0

pass() { echo "  ✅ PASS: $1"; ((PASS++)); }
fail() { echo "  ❌ FAIL: $1"; ((FAIL++)); }

echo "=== Tests Ejercicio 0.1 ==="

# Test 1: El binario existe
echo "--- Test 1: Binario existe ---"
if [ -x "$BINARY" ]; then
    pass "Binario existe y es ejecutable"
else
    fail "Binario no encontrado en $BINARY"
    echo "  Total: $PASS passed, $FAIL failed"
    exit 1
fi

# Test 2: Sale con código 0
echo "--- Test 2: Exit code ---"
if $BINARY > /dev/null 2>&1; then
    pass "Exit code es 0"
else
    fail "Exit code no es 0"
fi

# Test 3: Output contiene "Hello from"
echo "--- Test 3: Output format ---"
OUTPUT=$($BINARY 2>/dev/null)
if echo "$OUTPUT" | grep -q "^Hello from "; then
    pass "Output empieza con 'Hello from '"
else
    fail "Output no empieza con 'Hello from '. Got: '$OUTPUT'"
fi

# Test 4: Output contiene el nombre de la distro
echo "--- Test 4: Distro detection ---"
DISTRO_ID=$(grep "^ID=" /etc/os-release | cut -d= -f2 | tr -d '"')
if echo "$OUTPUT" | grep -qi "$DISTRO_ID"; then
    pass "Output contiene el nombre de la distro ($DISTRO_ID)"
else
    fail "Output no contiene '$DISTRO_ID'. Got: '$OUTPUT'"
fi

# Test 5: No hay comillas en el output
echo "--- Test 5: Sin comillas ---"
if echo "$OUTPUT" | grep -q '"'; then
    fail "Output contiene comillas sin limpiar"
else
    pass "Output no tiene comillas"
fi

echo ""
echo "=== Resultado: $PASS passed, $FAIL failed ==="
[ "$FAIL" -eq 0 ] || exit 1
