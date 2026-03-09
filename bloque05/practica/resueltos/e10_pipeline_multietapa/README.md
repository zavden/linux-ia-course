# E10 — pipeline multietapa con hilos

## Objetivo
Modelar un flujo por etapas con colas thread-safe y señal de cierre (sentinel).

## Qué hace
- Etapa 1 transforma `x -> 2x`.
- Etapa 2 transforma `y -> y + 1`.
- Agrega resultados finales y valida total.

## Ejecutar
```bash
make run
make test
```
