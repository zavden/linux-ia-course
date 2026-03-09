# E02 — argumentos por struct y retorno en join

## Objetivo
Pasar múltiples parámetros a un hilo usando `struct` y recuperar resultados con `pthread_join`.

## Qué hace
- Crea 3 hilos con rangos distintos.
- Cada hilo calcula suma de cuadrados en su rango.
- Devuelve el resultado en heap y `main` lo agrega.

## Ejecutar
```bash
make run
make test
```
