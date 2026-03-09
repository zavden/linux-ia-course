#!/bin/bash
set -u

echo "=== Tests Ejercicio A.4 (Backup Script) ==="
cd src/

if [ ! -x "./backup.sh" ]; then
    echo "❌ src/backup.sh no existe o no es ejecutable"
    exit 1
fi

# Test 1: No args
OUTPUT=$(./backup.sh 2>&1 || true)
if echo "$OUTPUT" | grep -qi "uso"; then
    echo "✅ Test 1 ok: Valida argumentos"
else
    echo "❌ Test 1 falló: No imprime mensaje de uso al faltar argumentos"
    exit 1
fi

# Test 2: Invalid dir
OUTPUT=$(./backup.sh "dir_falso_123" 2>&1 || true)
if echo "$OUTPUT" | grep -qi "error.*no existe"; then
    echo "✅ Test 2 ok: Valida existencia"
else
    echo "❌ Test 2 falló: No valida correctamente si el directorio existe"
    exit 1
fi

# Test 3: Valid dir
mkdir -p dummy_dir
echo "hello" > dummy_dir/f1.txt
./backup.sh dummy_dir > /dev/null
ARCHIVOS=$(ls backup_dummy_dir_*.tar.gz 2>/dev/null | wc -l)

if [ "$ARCHIVOS" -eq 1 ]; then
    echo "✅ Test 3 ok: Genera archivo .tar.gz correcto"
else
    echo "❌ Test 3 falló: No se generó archivo .tar.gz o el nombre es incorrecto"
    exit 1
fi

rm -rf dummy_dir backup_dummy_dir_*.tar.gz
echo "PASSED"
