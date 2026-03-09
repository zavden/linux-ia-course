# E06 — RLIMIT_NOFILE

## Objetivo
Consultar y ajustar límite de descriptores abiertos de un proceso.

## Qué hace
- Lee soft/hard de `RLIMIT_NOFILE`.
- Ajusta soft a un valor menor controlado.
- Verifica límite actualizado.

## Ejecutar
```bash
make run
make test
```
