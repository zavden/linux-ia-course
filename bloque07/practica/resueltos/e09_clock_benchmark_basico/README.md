# E09 — benchmark básico con clock_gettime

## Objetivo
Medir tiempo de ejecución de una sección de código de forma reproducible.

## Qué hace
- Ejecuta una carga sintética en loop.
- Mide duración con `CLOCK_MONOTONIC`.
- Reporta nanosegundos y valor acumulado.

## Ejecutar
```bash
make run
make test
```
