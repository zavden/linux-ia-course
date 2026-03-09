# C01 — argflow (CLI robusta)

## Objetivo
Diseñar un parser de argumentos de nivel utilitario real.

## Qué debe hacer
- Soportar `-v/--verbose`, `-o/--output`, `-n/--number`, `--dry-run`.
- Manejar `--` y argumentos posicionales correctamente.
- Validar `--number` con `strtol` + rango.
- Imprimir errores claros en `stderr`.
- Salir con códigos coherentes (`0` éxito, `!=0` error).

## Estado
Plantilla guiada (no resuelto).

## Ejecutar
```bash
make run
```
