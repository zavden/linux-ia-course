# E06 — cola con condvar y cierre limpio

## Objetivo
Coordinar múltiples productores/consumidores con cola compartida y finalización sin deadlock.

## Qué hace
- Usa una cola circular protegida por mutex.
- 2 productores insertan tareas.
- 3 consumidores drenan la cola y terminan al detectar cierre.

## Ejecutar
```bash
make run
make test
```
