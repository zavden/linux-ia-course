# E09 — constructor de inventario de storage

## Objetivo
Unificar datos de montajes, LVM y quotas en un resumen tipo inventario.

## Qué hace
- Reutiliza parsers simples sobre 3 archivos de entrada.
- Agrega métricas clave en una línea estilo JSON.
- Sirve como base para auditor o dashboard.

## Ejecutar
```bash
make run
make test
```
