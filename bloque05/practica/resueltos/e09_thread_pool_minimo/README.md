# E09 — thread pool mínimo

## Objetivo
Implementar un pool fijo de workers consumiendo tareas desde una cola compartida.

## Qué hace
- Crea 4 workers persistentes.
- Encola 20 tareas numéricas.
- Cada worker procesa y acumula suma de cuadrados.

## Ejecutar
```bash
make run
make test
```
