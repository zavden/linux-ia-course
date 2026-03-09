# E04 — mutex para contador seguro

## Objetivo
Corregir una sección crítica compartida usando `pthread_mutex_t`.

## Qué hace
- Lanza varios hilos que incrementan el mismo contador.
- Protege `counter++` con lock/unlock.
- Verifica que el resultado final sea exacto.

## Ejecutar
```bash
make run
make test
```
