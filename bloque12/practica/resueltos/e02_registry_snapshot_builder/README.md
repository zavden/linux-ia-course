# E02 - constructor de snapshot de registry

## Objetivo
Convertir estado de servicios en un resumen compacto util para discovery.

## Que hace
- Parsea lineas `service|host|port|status`.
- Cuenta servicios UP y DOWN.
- Emite salida JSON compacta con resumen.

## Ejecutar
```bash
make run
make test
```
