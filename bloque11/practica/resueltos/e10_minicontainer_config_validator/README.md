# E10 - validador de config de minicontenedor

## Objetivo
Validar configuración declarativa de runtime antes de intentar ejecutar aislamiento.

## Qué hace
- Parsea archivo de configuración `key=value`.
- Exige campos obligatorios (`rootfs`, `cmd`, `mem_max`, `cpu_max`, `namespaces`, `seccomp_profile`).
- Valida formato/rango y emite `valid=0/1`.

## Ejecutar
```bash
make run
make test
```
