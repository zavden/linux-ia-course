# E10 — minicache con event loop y poll

## Objetivo
Integrar parsing por líneas, almacenamiento key/value y multiplexación por `poll`.

## Qué hace
- Simula 2 clientes conectados al servidor (con `socketpair`).
- El servidor procesa comandos `SET/GET/DEL` en un loop reactor.
- Devuelve respuestas por cada cliente y valida protocolo.

## Ejecutar
```bash
make run
make test
```
