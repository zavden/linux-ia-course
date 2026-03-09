# E01 - parser de /proc/modules

## Objetivo
Extraer métricas básicas de módulos del kernel desde el formato de `/proc/modules`.

## Qué hace
- Lee líneas de módulos cargados.
- Cuenta módulos totales y cuántos están en uso.
- Cuenta dependencias declaradas en columna `deps`.

## Ejecutar
```bash
make run
make test
```
