# E01 — pthread create/join básico

## Objetivo
Crear varios hilos, repartir trabajo independiente y esperar su finalización con `pthread_join`.

## Qué hace
- Divide un arreglo en 4 segmentos.
- Cada hilo suma su tramo.
- El hilo principal junta los resultados parciales y valida el total.

## Ejecutar
```bash
make run
make test
```
