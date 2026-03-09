# E05 - parser de memoria en cgroup v2

## Objetivo
Interpretar consumo y límites de memoria usando archivos de cgroup v2.

## Qué hace
- Lee `memory.max`, `memory.current` y `memory.swap.max`.
- Detecta si límites están activos o en `max`.
- Calcula porcentaje de uso sobre límite principal.

## Ejecutar
```bash
make run
make test
```
