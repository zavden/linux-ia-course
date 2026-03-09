# E07 - parser de /etc/exports (NFS)

## Objetivo
Entender como leer reglas de exportacion NFS y extraer postura de acceso.

## Que hace
- Parsea lineas de `exports` con multiples clientes por share.
- Cuenta hosts `rw` y `ro`.
- Detecta uso de opcion `no_root_squash`.

## Ejecutar
```bash
make run
make test
```
