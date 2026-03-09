# C01 — allocator segmentado (guiado)

## Objetivo
Diseñar un allocator con múltiples tamaños de bloque para reducir fragmentación.

## Qué debe hacer
- Segregar pools por clases de tamaño (ej. 32, 64, 128 bytes).
- `alloc/free` por clase con listas libres.
- fallback a `malloc` para bloques grandes.

## Estado
Plantilla guiada (sin resolver completamente).
