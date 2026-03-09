# E10 — mini auditor de almacenamiento

## Objetivo
Integrar chequeos de capacidad y policy en un reporte de salud de storage.

## Qué hace
- Mide uso de filesystem con `statvfs`.
- Lee reporte de quotas y cuenta usuarios en riesgo.
- Emite `status=OK|WARN|CRIT` según umbrales.

## Ejecutar
```bash
make run
make test
```
