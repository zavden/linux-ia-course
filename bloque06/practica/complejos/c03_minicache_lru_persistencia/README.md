# C03 — minicache con LRU y persistencia

## Objetivo
Evolucionar un cache key/value en memoria con expulsión LRU y snapshot en disco.

## Qué debe hacer
- Límite de capacidad configurable (número de claves).
- Política LRU para expulsar al insertar cuando está lleno.
- Comandos `SAVE` y `LOAD` sobre archivo local.
- Mantener reactor single-thread para clientes concurrentes.

## Pistas
- Estructura recomendada: hash map + lista doble enlazada para LRU.
- Persistencia inicial: formato texto simple, una clave/valor por línea.
- Valida integridad y errores de parseo en `LOAD`.
