# E09 — backpressure en socket no bloqueante

## Objetivo
Detectar saturación de buffer de envío y reintentar tras drenar receptor.

## Qué hace
- Marca escritor como `O_NONBLOCK`.
- Escribe hasta recibir `EAGAIN` (buffer lleno).
- Drena receptor y vuelve a escribir.

## Ejecutar
```bash
make run
make test
```
