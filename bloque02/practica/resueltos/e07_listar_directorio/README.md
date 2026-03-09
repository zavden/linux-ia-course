# E07 — Listado simple de directorio

## Objetivo
Recorrer un directorio con `opendir/readdir` y mostrar tipo+tamaño por entrada.

## Qué hace
- Recibe una ruta de directorio.
- Ignora `.` y `..`.
- Usa `lstat` para clasificar sin seguir symlink.

## Ejecutar
```bash
make run
make test
```
