# E08 — semáforo como rate limiter

## Objetivo
Limitar concurrencia máxima con semáforo contador.

## Qué hace
- Lanza 10 hilos de "petición".
- Solo permite 3 en sección crítica al mismo tiempo.
- Incluye fallback a semáforo nombrado si `sem_init` no está disponible.

## Ejecutar
```bash
make run
make test
```
