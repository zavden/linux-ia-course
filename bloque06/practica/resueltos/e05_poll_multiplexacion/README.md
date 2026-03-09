# E05 — multiplexación con poll

## Objetivo
Usar `poll` para observar múltiples FDs con estructura más flexible que `select`.

## Qué hace
- Crea dos pipes.
- Activa datos solo en uno.
- Verifica `revents` con `POLLIN`.

## Ejecutar
```bash
make run
make test
```
