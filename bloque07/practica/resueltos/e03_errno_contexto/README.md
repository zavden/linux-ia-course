# E03 — contexto de errores con errno

## Objetivo
Propagar y reportar errores de sistema con contexto útil para depuración.

## Qué hace
- Intenta abrir un archivo inexistente.
- Lee `errno` y lo traduce a nombre legible.
- Imprime mensaje contextual reproducible para logs.

## Ejecutar
```bash
make run
make test
```
