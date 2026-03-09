# E04 - parser de CPU en cgroup v2

## Objetivo
Leer límites de CPU en cgroup v2 desde `cpu.max` y `cpu.weight`.

## Qué hace
- Parsea `cpu.max` (`quota period` o `max period`).
- Detecta si hay límite efectivo.
- Calcula porcentaje teórico de CPU asignada.

## Ejecutar
```bash
make run
make test
```
