# E06 — watcher de filesystem (inotify + fallback)

## Objetivo
Detectar cambios en un directorio de forma programática.

## Qué hace
- En Linux usa `inotify` para evento de creación.
- En otras plataformas usa fallback por sondeo (`stat` + `access`).
- Reporta método y detección del cambio.

## Ejecutar
```bash
make run
make test
```
