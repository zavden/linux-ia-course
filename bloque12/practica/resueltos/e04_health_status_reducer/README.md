# E04 - reductor de estados de salud

## Objetivo
Consolidar multiples señales de salud en una severidad unica.

## Que hace
- Lee lineas `service=status` con `ok/warn/crit`.
- Calcula estado global por prioridad.
- Reporta servicios en riesgo.

## Ejecutar
```bash
make run
make test
```
