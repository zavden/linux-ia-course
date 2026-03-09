# C02 — daemon de producción (guiado)

## Objetivo
Evolucionar daemon básico hacia servicio robusto con pidfile y señales.

## Qué debe hacer
- Exclusión por pidfile.
- Manejar `SIGTERM` (shutdown limpio) y `SIGHUP` (reload).
- Logging periódico a archivo.
- Limpieza garantizada al terminar.

## Estado
Plantilla guiada (sin resolver completamente).
