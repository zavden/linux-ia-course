# E10 - agregador de salud de servicios de red

## Objetivo
Consolidar estado de multiples servicios en una sola severidad operativa.

## Que hace
- Lee archivo `servicio=estado` (`ok`, `warn`, `crit`).
- Cuenta servicios por nivel.
- Emite estado global `OK/WARN/CRIT`.

## Ejecutar
```bash
make run
make test
```
