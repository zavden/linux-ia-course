# E01 — getopt_long básico

## Objetivo
Construir una CLI mínima robusta que acepte opciones cortas y largas.

## Qué hace
- Soporta `-v/--verbose`, `-n/--number`, `-h/--help`.
- Valida `--number` con `strtol` (no usa `atoi`).
- Imprime configuración final parseada.

## Ejecutar
```bash
make run
make test
```
