# E04 - parser de URL HTTPS

## Objetivo
Descomponer una URL HTTPS en esquema, host, puerto y path.

## Que hace
- Valida esquema `https://`.
- Extrae host y puerto (o usa 443 por defecto).
- Devuelve path normalizado.

## Ejecutar
```bash
make run
make test
```
