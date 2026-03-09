# E05 — safe_snprintf

## Objetivo
Envolver `vsnprintf` para manejar formato variádico con control de truncamiento.

## Qué hace
- Implementa wrapper `safe_snprintf`.
- Mide si la salida fue truncada.
- Evita `sprintf` inseguro.

## Ejecutar
```bash
make run
make test
```
