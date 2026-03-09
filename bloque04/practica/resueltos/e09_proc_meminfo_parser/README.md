# E09 — Parser de meminfo

## Objetivo
Parsear métricas de memoria desde formato tipo `/proc/meminfo`.

## Qué hace
- Lee archivo de entrada (`/proc/meminfo` por defecto si existe).
- Extrae `MemTotal`, `MemFree`, `Buffers`, `Cached`.
- Calcula memoria usada aproximada.

## Ejecutar
```bash
make run
make test
```
