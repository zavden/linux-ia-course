#!/bin/bash
set -euo pipefail
echo "=== Tests Proyecto 02 (minifind) ==="

cd solucion/

# Compilamos usando el Makefile
make clean >/dev/null
make >/dev/null

# ---- PREPARACION ENTORNO ----
mkdir -p test_tree/a/b/c
touch test_dir/archivo.txt
touch test_tree/a/b/c/secreto.txt
touch test_tree/a/b/c/gigante.bin
# Llenamos uno de 20 bytes
echo "0123456789012345678" > test_tree/a/b/c/gigante.bin

echo ">> Testeando Recursividad..."

OUT=$(./minifind test_tree)

if echo "$OUT" | grep -q "secreto\.txt"; then
    echo "✅ Encontró secreto.txt en las profundidades."
else
    echo "❌ Falló el escáner recursivo básico."
    exit 1
fi

echo ">> Testeando filtro -name..."
OUT=$(./minifind test_tree -name secreto.txt)

if echo "$OUT" | grep -q "gigante"; then
    echo "❌ Trajo gigante.bin cuando solo pedimos secreto.txt."
    exit 1
fi

echo ">> Testeando filtro -type d..."
OUT=$(./minifind test_tree -type d)

if echo "$OUT" | grep -q "secreto\.txt"; then
    echo "❌ Trajo un archivo cuando solo pedimos directorios (-type d)."
    exit 1
fi
if echo "$OUT" | grep -q "test_tree/a"; then
    echo "✅ Detectó directorios correctamente."
else
    echo "❌ Falló detectando directorios."
    exit 1
fi

echo ">> Testeando filtro -size..."
OUT=$(./minifind test_tree -size 20c)

if echo "$OUT" | grep -v "gigante\.bin" | grep -q "."; then
    echo "❌ Trajo cosas que no pesaban 20c exactos."
    exit 1
fi
if echo "$OUT" | grep -q "gigante\.bin"; then
    echo "✅ Filtro por Bytes Exactos Funciona."
else
    echo "❌ No encontró gigante.bin sabiendo que pesa 20 bytes."
    exit 1
fi

rm -rf test_dir test_tree minifind

echo "✅ Tu clon de find navega y filtra como un profesional."
echo "PASSED"
