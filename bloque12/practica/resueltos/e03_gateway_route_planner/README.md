# E03 - planificador de rutas para gateway

## Objetivo
Resolver rutas HTTP a backends validos en base a estado de registry.

## Que hace
- Parsea definicion de backends y rutas por prefijo.
- Aplica longest-prefix-match.
- Solo enruta a backends en estado `up`.

## Ejecutar
```bash
make run
make test
```
