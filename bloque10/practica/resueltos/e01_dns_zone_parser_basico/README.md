# E01 - parser basico de zona DNS

## Objetivo
Aprender a extraer registros clave de un archivo de zona estilo BIND.

## Que hace
- Lee lineas de una zona DNS y omite comentarios/directivas.
- Detecta tipos `A`, `AAAA`, `CNAME`, `MX`, `NS`, `SOA`, `TXT`.
- Reporta conteo total y por tipo.

## Ejecutar
```bash
make run
make test
```
