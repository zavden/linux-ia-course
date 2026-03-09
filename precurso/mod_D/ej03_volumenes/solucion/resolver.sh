#!/bin/bash
# Solución al Ejercicio D.3

docker run --rm -v "$PWD/src":/usr/src/miapp -w /usr/src/miapp gcc:latest gcc -o hola hola.c
echo "Revisa la carpeta src/. El ejecutable 'hola' deberia haber aparecido."
