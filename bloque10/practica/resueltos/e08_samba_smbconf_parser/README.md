# E08 - parser de smb.conf (Samba)

## Objetivo
Interpretar configuracion de shares Samba para auditoria basica.

## Que hace
- Lee secciones de `smb.conf`.
- Cuenta shares (excluyendo `[global]`).
- Calcula shares escribibles, con `guest ok` y no browseables.

## Ejecutar
```bash
make run
make test
```
