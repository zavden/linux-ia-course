# E04 — multiplexación con select

## Objetivo
Esperar actividad en múltiples descriptores sin bloquear en uno solo.

## Qué hace
- Crea dos pipes.
- Escribe datos solo en uno.
- Usa `select` para detectar cuál quedó listo para lectura.

## Ejecutar
```bash
make run
make test
```
