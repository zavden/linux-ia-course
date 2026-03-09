# E02 — TCP loopback: bind/listen/accept

## Objetivo
Montar servidor TCP mínimo con cliente local para practicar ciclo completo de conexión.

## Qué hace
- Crea socket servidor en `127.0.0.1` puerto dinámico.
- Cliente hijo conecta y envía `hola_tcp`.
- Servidor acepta y responde `ok:hola_tcp`.

## Ejecutar
```bash
make run
make test
```
