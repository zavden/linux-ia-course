# E09 — daemon básico (doble fork)

## Objetivo
Ejecutar un proceso en segundo plano desacoplado de la terminal.

## Qué hace
- Aplica patrón de daemonización clásico.
- Escribe 3 líneas en `/tmp/e09_daemon.log`.
- Termina solo (no se queda residente indefinidamente).

## Ejecutar
```bash
make run
make test
```
