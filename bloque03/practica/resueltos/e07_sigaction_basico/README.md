# E07 — sigaction básico

## Objetivo
Manejar señales de forma controlada usando flags `sig_atomic_t`.

## Qué hace
- `SIGUSR1`: incrementa contador.
- `SIGTERM`: solicita salida limpia.
- Loop principal imprime estado hasta recibir término.

## Ejecutar
```bash
make run
make test
```
