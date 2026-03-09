# E07 — rwlock para caché compartida

## Objetivo
Permitir lecturas concurrentes y escrituras exclusivas con `pthread_rwlock_t`.

## Qué hace
- Lanza 4 lectores y 2 escritores.
- Lectores usan `rdlock`, escritores usan `wrlock`.
- Verifica valor final y contadores de operaciones.

## Ejecutar
```bash
make run
make test
```
