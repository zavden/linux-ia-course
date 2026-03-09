# E03 — safe_strcpy

## Objetivo
Implementar copia segura con semántica estilo `strlcpy`.

## Qué hace
- Copia con tamaño máximo.
- Garantiza `\0` si `dst_size > 0`.
- Retorna longitud original para detectar truncamiento.

## Ejecutar
```bash
make run
make test
```
