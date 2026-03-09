# E04 — umask y modo final de creación

## Objetivo
Entender cómo `umask` afecta los permisos al crear archivos.

## Qué hace
- Recibe `<archivo> <modo_octal>`.
- Aplica `umask(0)` temporal para respetar modo exacto.
- Crea archivo y muestra modo final observado con `stat`.

## Ejecutar
```bash
make run
make test
```
