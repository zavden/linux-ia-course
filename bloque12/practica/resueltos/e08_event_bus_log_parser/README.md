# E08 - parser de logs de event bus

## Objetivo
Extraer senales de operacion desde eventos de integracion entre servicios.

## Que hace
- Parsea lineas `ts service event result latency_ms`.
- Cuenta eventos `publish`, `consume`, `retry`.
- Detecta errores y latencia maxima observada.

## Ejecutar
```bash
make run
make test
```
