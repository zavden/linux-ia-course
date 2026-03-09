# E07 — RLIMIT_AS (o fallback)

## Objetivo
Aplicar límite de memoria virtual y observar comportamiento del proceso.

## Qué hace
- Muestra límite actual.
- Intenta reducir soft limit.
- Reserva bloques de memoria hasta fallo o umbral.

## Ejecutar
```bash
make run
make test
```
