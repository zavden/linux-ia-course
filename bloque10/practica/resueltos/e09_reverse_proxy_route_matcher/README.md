# E09 - matcher de rutas para reverse proxy

## Objetivo
Resolver backend destino usando tabla de rutas por prefijo.

## Que hace
- Parsea config con `backend` y `route`.
- Aplica estrategia de **longest prefix match**.
- Mapea path entrante a `backend -> host:port`.

## Ejecutar
```bash
make run
make test
```
