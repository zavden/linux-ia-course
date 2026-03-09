# E07 — parser de reporte LVM

## Objetivo
Parsear salidas tabulares de LVM para extraer métricas útiles.

## Qué hace
- Lee archivo estilo `lvs --separator ';'`.
- Cuenta volúmenes lógicos y detecta thin volumes.
- Suma tamaño total en MiB aproximados.

## Ejecutar
```bash
make run
make test
```
