# E03 — race condition demostrable

## Objetivo
Entender por qué `counter++` no es atómico cuando varios hilos lo ejecutan sin sincronización.

## Qué hace
- Ejecuta dos hilos sobre el mismo contador global.
- Fuerza intercalado de pasos lectura/escritura.
- Muestra diferencia entre `expected` y `observed`.

## Ejecutar
```bash
make run
make test
```
