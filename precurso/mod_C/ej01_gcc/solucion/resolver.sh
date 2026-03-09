#!/bin/bash
# Solución al Ejercicio C.1 — Compilación Manual

# 1. Objeto math.o
gcc -Wall -c src/math.c

# 2. Objeto main.o
gcc -Wall -c src/main.c

# 3. Mover temporales
mv main.o src/
mv math.o src/

# 4. Compilar ejecutable
gcc -o src/calculadora src/main.o src/math.o

echo "Compilación exitosa. Ejecuta ./src/calculadora para probarlo."
