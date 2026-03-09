# E02 — realloc seguro

## Objetivo
Aplicar patrón de realocación segura sin perder puntero original.

## Qué hace
- Crea buffer inicial.
- Intenta realocar a tamaño mayor.
- Conserva estado válido si la realocación falla.

## Ejecutar
```bash
make run
make test
```
