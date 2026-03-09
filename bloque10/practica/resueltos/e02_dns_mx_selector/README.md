# E02 - selector de MX preferido

## Objetivo
Interpretar prioridades de registros MX para elegir servidor de correo primario.

## Que hace
- Lee registros `MX` desde archivo de zona simplificado.
- Filtra por dominio objetivo.
- Elige host con menor preferencia numerica.

## Ejecutar
```bash
make run
make test
```
