# E05 - parser de line protocol de metricas

## Objetivo
Procesar eventos de metricas simples para generar resumen de latencia y error rate.

## Que hace
- Lee lineas `service latency_ms status`.
- Calcula promedio de latencia.
- Cuenta requests con estado de error (`>=500`).

## Ejecutar
```bash
make run
make test
```
