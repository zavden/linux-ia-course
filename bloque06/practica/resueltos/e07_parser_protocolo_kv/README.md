# E07 — parser de protocolo key/value

## Objetivo
Implementar parseo textual de comandos `SET/GET/DEL` con almacenamiento en memoria.

## Qué hace
- Usa un array fijo como base de datos simple.
- Procesa comandos por línea.
- Devuelve respuestas estables (`OK`, `VALUE`, `NULL`, `ERR`).

## Ejecutar
```bash
make run
make test
```
