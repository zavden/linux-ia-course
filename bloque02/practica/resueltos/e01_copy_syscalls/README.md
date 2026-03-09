# E01 — Copia robusta con syscalls

## Objetivo
Implementar una copia de archivos binarios usando `open/read/write/close` con manejo correcto de errores.

## Qué hace
- Recibe `origen` y `destino`.
- Lee por bloques de 4 KiB.
- Escribe manejando posibles escrituras parciales.

## Ejecutar
```bash
make run
make test
```
