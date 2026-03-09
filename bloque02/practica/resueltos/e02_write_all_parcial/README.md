# E02 — Escritura parcial controlada

## Objetivo
Comprender y resolver escrituras parciales implementando un `write_all` reusable.

## Qué hace
- Copia archivo de texto.
- Fuerza escrituras de máximo 3 bytes por syscall.
- Demuestra que el loop de `write_all` reconstruye todo el contenido.

## Ejecutar
```bash
make run
make test
```
