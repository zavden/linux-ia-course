# E02 — planner de drop de capabilities

## Objetivo
Calcular qué capabilities conservar y cuáles eliminar según una política mínima.

## Qué hace
- Parte de una máscara `current`.
- Define una máscara `required`.
- Calcula `keep` y `drop`.
- Reporta cantidad de bits en cada conjunto.

## Ejecutar
```bash
make run
make test
```
