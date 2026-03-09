# E06 — framing de líneas

## Objetivo
Separar mensajes lógicos por delimitador (`\n`) aunque lleguen en chunks parciales.

## Qué hace
- Alimenta buffer incrementalmente.
- Extrae líneas completas sin perder fragmentos.
- Demuestra parser robusto ante cortes arbitrarios.

## Ejecutar
```bash
make run
make test
```
