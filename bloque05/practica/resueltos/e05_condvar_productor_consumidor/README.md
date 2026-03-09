# E05 — condvar productor/consumidor

## Objetivo
Dormir consumidores sin busy-wait usando `pthread_cond_wait`.

## Qué hace
- Productor genera 20 items.
- Consumidor espera cuando no hay stock.
- Ambos coordinan con mutex + condvars.

## Ejecutar
```bash
make run
make test
```
