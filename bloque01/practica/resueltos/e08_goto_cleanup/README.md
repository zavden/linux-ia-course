# E08 — Patrón goto cleanup

## Objetivo
Unificar la liberación de recursos ante múltiples puntos de fallo.

## Qué hace
- Reserva memoria.
- Intenta abrir archivo inexistente.
- Usa bloque único `cleanup` para liberar en orden seguro.

## Ejecutar
```bash
make run
make test
```
