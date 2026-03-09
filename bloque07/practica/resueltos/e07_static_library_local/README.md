# E07 — librería estática local (`.a`)

## Objetivo
Entender el flujo de compilación por objetos y empaquetado en librería estática.

## Qué hace
- Compila `text_stats.c` a objeto.
- Construye `libtextstats.a` con `ar`.
- Linkea `main` contra la librería y ejecuta cálculo textual.

## Ejecutar
```bash
make run
make test
```
