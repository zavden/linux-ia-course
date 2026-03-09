# E07 - validador de job spec para runner

## Objetivo
Verificar especificaciones de ejecucion antes de enviar trabajos al runner.

## Que hace
- Parsea lineas `job|image|cpu_milli|mem_mb|timeout_sec`.
- Valida limites operativos minimos y maximos.
- Cuenta jobs validos y jobs de alto consumo.

## Ejecutar
```bash
make run
make test
```
