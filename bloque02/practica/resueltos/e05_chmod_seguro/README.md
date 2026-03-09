# E05 — chmod/fchmod seguro

## Objetivo
Aplicar cambios de permisos sobre archivo existente con confirmación por `fstat`.

## Qué hace
- Recibe `<archivo> <modo_octal>`.
- Abre archivo y aplica `fchmod` sobre FD abierto.
- Verifica y muestra modo resultante.

## Ejecutar
```bash
make run
make test
```
