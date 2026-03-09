# E03 — extended attributes (xattr) básico

## Objetivo
Aplicar operaciones de atributo extendido sobre archivo: set/get/list/remove.

## Qué hace
- Crea archivo temporal.
- Escribe y lee un xattr.
- Lista atributos y elimina el atributo creado.
- Incluye fallback sidecar si xattr no está soportado en el FS actual.

## Ejecutar
```bash
make run
make test
```
