# E01 — parser de capabilities en /proc/status

## Objetivo
Interpretar máscaras de capabilities Linux en formato hexadecimal.

## Qué hace
- Lee línea `CapEff` desde un archivo tipo `/proc/self/status`.
- Convierte máscara hex a entero.
- Reporta si están activas capacidades concretas.

## Ejecutar
```bash
make run
make test
```
