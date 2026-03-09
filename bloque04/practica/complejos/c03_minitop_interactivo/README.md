# C03 — minitop interactivo (guiado)

## Objetivo
Evolucionar `minitop` a monitor continuo con refresco, orden y filtros.

## Qué debe hacer
- Loop de refresco con intervalo configurable.
- Parsear `/proc/meminfo`, `/proc/loadavg`, `/proc/<pid>/status|stat`.
- Ordenar por RSS descendente.
- Opción para mostrar top N procesos.
- Manejar errores por PID sin caer.

## Estado
Plantilla guiada (sin resolver completamente).
