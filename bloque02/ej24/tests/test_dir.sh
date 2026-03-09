#!/bin/bash
set -euo pipefail
echo "=== Tests Ejercicio 2.4 (Directorios y stat) ==="

cd solucion/src

# Compilando
gcc -O2 -Wall -Wextra -std=c17 main.c -o my_ls || exit 1

# Creamos entorno de test
mkdir -p test_dir
touch test_dir/archivo_vacio.txt
echo "12345" > test_dir/cinco_bytes.txt
ln -s cinco_bytes.txt test_dir/enlace.sym

# Ejecutamos 
OUT=$(./my_ls test_dir)
echo "$OUT"

if echo "$OUT" | grep -q "0.*-.*archivo_vacio\.txt"; then
    echo "✅ Detectó archivo regular vacío (Size 0)"
else
    echo "❌ Falló detectando archivo_vacio.txt"
    exit 1
fi

if echo "$OUT" | grep -q "6.*-.*cinco_bytes\.txt"; then
    echo "✅ Detectó archivo regular con 6 bytes (5 chars + \n)"
else
    echo "❌ Falló leyendo el st_size correcto de cinco_bytes.txt"
    exit 1
fi

if echo "$OUT" | grep -q "l.*enlace\.sym"; then
    echo "✅ Detectó que el symlink es de tipo 'l' (link) y no resolvió su target (usó lstat, no stat)"
else
    echo "❌ Falló detectando el tipo l (symlink) o usó stat() en lugar de lstat()."
    exit 1
fi

rm -rf test_dir my_ls
echo "PASSED"
