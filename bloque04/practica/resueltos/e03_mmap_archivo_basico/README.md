# E03 — mmap de archivo básico

## Objetivo
Mapear un archivo en memoria, modificar bytes y sincronizar cambios.

## Qué hace
- Abre archivo en `O_RDWR`.
- Lo mapea con `MAP_SHARED`.
- Modifica primer byte y llama `msync`.

## Ejecutar
```bash
make run
make test
```
