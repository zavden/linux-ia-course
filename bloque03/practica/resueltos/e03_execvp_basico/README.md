# E03 — fork + execvp básico

## Objetivo
Reemplazar la imagen del proceso hijo para ejecutar un comando externo.

## Qué hace
- Ejecuta comando recibido por CLI.
- Usa `fork` + `execvp`.
- Padre espera y reporta estado final.

## Ejecutar
```bash
make run
make test
```
