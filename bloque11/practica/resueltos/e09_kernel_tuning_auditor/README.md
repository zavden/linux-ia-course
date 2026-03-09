# E09 - auditor de tuning de kernel

## Objetivo
Evaluar configuración kernel contra una política mínima de hardening.

## Qué hace
- Parsea archivo `key=value` con parámetros kernel.
- Verifica reglas exactas y una regla de umbral (`vm.swappiness`).
- Emite resumen `OK/WARN/CRIT`.

## Ejecutar
```bash
make run
make test
```
