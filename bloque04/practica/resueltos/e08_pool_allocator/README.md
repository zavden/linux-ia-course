# E08 — Pool allocator lineal

## Objetivo
Construir un allocator O(1) por desplazamiento (`bump allocator`).

## Qué hace
- Reserva bloque grande único.
- Entrega sub-bloques por offset creciente.
- Libera todo con un solo `free` al final.

## Ejecutar
```bash
make run
make test
```
