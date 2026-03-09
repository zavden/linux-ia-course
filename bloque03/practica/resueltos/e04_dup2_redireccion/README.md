# E04 — redirección con dup2

## Objetivo
Redirigir `stdout` de un comando hacia un archivo destino.

## Qué hace
- Uso: `./main <archivo_salida> <cmd> [args...]`.
- Hijo abre archivo, hace `dup2(fd, STDOUT_FILENO)`, ejecuta comando.
- Padre espera finalización.

## Ejecutar
```bash
make run
make test
```
