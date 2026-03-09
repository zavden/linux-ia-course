# E03 - decoder de flags de namespaces

## Objetivo
Interpretar máscara de flags `CLONE_NEW*` para saber qué aislamientos se solicitan.

## Qué hace
- Lee máscara hexadecimal de flags.
- Decodifica `mnt`, `uts`, `ipc`, `user`, `pid`, `net`, `cgroup`.
- Cuenta cuántos namespaces nuevos fueron pedidos.

## Ejecutar
```bash
make run
make test
```
