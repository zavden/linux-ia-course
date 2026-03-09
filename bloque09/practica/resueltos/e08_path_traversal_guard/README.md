# E08 - guardia contra path traversal

## Objetivo
Evitar escapes de directorio base al componer rutas con input externo.

## Que hace
- Normaliza una ruta relativa (`.` y `..`).
- Rechaza rutas absolutas o segmentos inseguros.
- Construye ruta final solo si queda dentro de la raiz logica.

## Ejecutar
```bash
make run
make test
```
