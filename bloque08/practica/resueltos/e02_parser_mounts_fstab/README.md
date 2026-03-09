# E02 — parser de mounts y fstab

## Objetivo
Leer y parsear información de montajes desde archivos tipo `/proc/mounts` y `/etc/fstab`.

## Qué hace
- Ignora líneas vacías y comentarios.
- Cuenta entradas válidas en cada archivo.
- Reporta primer mountpoint detectado de cada fuente.

## Ejecutar
```bash
make run
make test
```
