# E04 — safe_strcat

## Objetivo
Concatenar strings sin desbordar el buffer destino.

## Qué hace
- Calcula longitud actual de `dst` con límite.
- Concatena solo el espacio disponible.
- Permite detectar truncamiento con el retorno.

## Ejecutar
```bash
make run
make test
```
