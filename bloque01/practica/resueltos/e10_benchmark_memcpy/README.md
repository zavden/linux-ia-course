# E10 — Benchmark básico de memoria

## Objetivo
Comparar una implementación ingenua de copia contra `memcpy` de libc.

## Qué hace
- Implementa `my_memcpy` byte a byte.
- Mide tiempos con `clock_gettime(CLOCK_MONOTONIC)`.
- Imprime comparación en milisegundos.

## Ejecutar
```bash
make run
make test
```
