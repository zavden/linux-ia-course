# E09 — Macro CHECK para syscalls

## Objetivo
Reducir boilerplate al verificar llamadas que retornan `-1` y setean `errno`.

## Qué hace
- Define `CHECK_SYS(call)` con archivo/línea.
- Forza un fallo de `open()` para ver el diagnóstico.
- Salta a `cleanup` en error.

## Ejecutar
```bash
make run
make test
```
