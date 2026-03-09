# E02 — waitpid no bloqueante

## Objetivo
Aprender a usar `WNOHANG` para no bloquear el proceso padre.

## Qué hace
- Crea un hijo que duerme y termina.
- El padre hace polling con `waitpid(..., WNOHANG)`.
- Muestra transición de "esperando" a "recolectado".

## Ejecutar
```bash
make run
make test
```
