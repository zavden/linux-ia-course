# E10 - parser de registros de minivault

## Objetivo
Parsear y validar un formato simple de metadatos de secretos.

## Que hace
- Lee lineas tipo `user|salt_hex|mac_hex|flags`.
- Valida formato de cada campo y cuenta registros invalidos.
- Reporta cantidad de usuarios validos y perfil admin.

## Ejecutar
```bash
make run
make test
```
