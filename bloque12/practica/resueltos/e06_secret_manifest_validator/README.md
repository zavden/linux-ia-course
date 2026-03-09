# E06 - validador de manifest de secretos

## Objetivo
Validar metadatos de secretos antes de publicarlos en el vault del sistema.

## Que hace
- Parsea lineas `secret|scope|ttl_sec|rotatable`.
- Verifica formato de nombre, scope permitido y TTL minimo.
- Cuenta secretos validos, globales y rotables.

## Ejecutar
```bash
make run
make test
```
