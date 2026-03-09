# C01 - minivault CLI secure storage (skeleton)

## Objetivo
Construir una CLI de almacenamiento seguro para secretos con enfoque en hardening y trazabilidad.

## Que debe hacer
- Alta y lectura de secretos con metadatos validados.
- Integridad de registros (HMAC) y proteccion de claves.
- Politicas de acceso por usuario/rol.
- Modo auditoria con salida estructurada.

## Pistas
- Separa capas: parser, crypto, storage, policy.
- Disena formato de archivo versionado y verificable.
- Implementa modo dry-run para operaciones sensibles.
