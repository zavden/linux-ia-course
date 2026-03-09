# C01 — cp robusto de producción (guiado)

## Objetivo
Construir un clon reducido de `cp` con manejo serio de errores de I/O.

## Qué debe hacer
- Copia binaria exacta `origen -> destino`.
- Soporta escrituras parciales y `EINTR`.
- Falla limpio con diagnóstico contextual.
- Mantiene permisos destino configurables.

## Estado
Plantilla guiada (sin resolver completamente).
