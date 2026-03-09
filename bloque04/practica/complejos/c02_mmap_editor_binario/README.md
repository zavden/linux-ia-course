# C02 — editor binario con mmap (guiado)

## Objetivo
Construir una utilidad para editar offsets de archivo usando mapeo de memoria.

## Qué debe hacer
- CLI: `<archivo> <offset> <byte_hex>`.
- Validar offset dentro de rango.
- Mapear con `MAP_SHARED`, escribir byte, `msync`, `munmap`.
- Manejar archivos vacíos y errores de formato.

## Estado
Plantilla guiada (sin resolver completamente).
