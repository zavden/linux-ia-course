#!/bin/bash
# Ejercicio A.4 (Solucion)

if [ $# -eq 0 ]; then
    echo "Uso: ./backup.sh <directorio>"
    exit 1
fi

DIR_TARGET="$1"

if [ ! -d "$DIR_TARGET" ]; then
    echo "Error: El directorio no existe"
    exit 1
fi

BASE_NAME=$(basename "$DIR_TARGET")
FECHA=$(date +%Y%m%d_%H%M%S)
ARCHIVO_SALIDA="backup_${BASE_NAME}_${FECHA}.tar.gz"

tar -czvf "$ARCHIVO_SALIDA" "$DIR_TARGET"

echo "Backup completado: $ARCHIVO_SALIDA"
