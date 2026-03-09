# E05 — df simplificado por rutas

## Objetivo
Construir una vista tipo `df` consultando uso de filesystem para múltiples rutas.

## Qué hace
- Toma lista de rutas por argumentos.
- Para cada ruta calcula total/avail/used% con `statvfs`.
- Resume cuántas rutas se procesaron correctamente.

## Ejecutar
```bash
make run
make test
```
