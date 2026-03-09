# E08 - inspector de namespaces por PID (mock)

## Objetivo
Leer representación textual de namespaces y construir un inventario utilizable.

## Qué hace
- Parsea líneas estilo `name:[inode]`.
- Cuenta namespaces presentes y detecta claves (`user`, `time`).
- Calcula cantidad de inodes únicos.

## Ejecutar
```bash
make run
make test
```
