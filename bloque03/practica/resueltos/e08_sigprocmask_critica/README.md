# E08 — sección crítica con sigprocmask

## Objetivo
Bloquear temporalmente señales para proteger una región crítica.

## Qué hace
- Bloquea `SIGUSR1`.
- Lanza `raise(SIGUSR1)` dentro de la sección bloqueada.
- Verifica que quedó pendiente y se entrega al desbloquear.

## Ejecutar
```bash
make run
make test
```
