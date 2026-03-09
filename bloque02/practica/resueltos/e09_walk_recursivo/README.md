# E09 — Recorrido recursivo de árbol de directorios

## Objetivo
Construir un walker recursivo seguro usando `opendir/readdir/lstat`.

## Qué hace
- Recorre todo el árbol desde una base.
- Imprime cada ruta visitada.
- No sigue symlinks para evitar ciclos.

## Ejecutar
```bash
make run
make test
```
