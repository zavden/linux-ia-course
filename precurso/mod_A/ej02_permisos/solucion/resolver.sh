#!/bin/bash
# Solución del Ejercicio A.2 — Permisos

# Crear archivos
touch src/secreto.sh
touch src/config.cfg

# Asignar permisos
# 700 = rwx para el dueño
chmod 700 src/secreto.sh

# 400 = solo lectura para el dueño
chmod 400 src/config.cfg

echo "Permisos asignados. Usa 'ls -l src/' para comprobar."
