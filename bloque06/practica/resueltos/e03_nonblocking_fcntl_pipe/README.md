# E03 — non-blocking con fcntl

## Objetivo
Aplicar `O_NONBLOCK` y manejar correctamente `EAGAIN/EWOULDBLOCK`.

## Qué hace
- Crea un `pipe`.
- Marca el extremo de lectura como no bloqueante.
- Demuestra lectura sin datos (`EAGAIN`) y lectura posterior con datos.

## Ejecutar
```bash
make run
make test
```
