# E07 - parser de sysctl.conf

## Objetivo
Practicar parseo de configuración kernel `key=value` con conteos por dominio.

## Qué hace
- Lee pares `key=value` ignorando comentarios y líneas vacías.
- Cuenta llaves `net.*`, `vm.*`, `kernel.*`.
- Señala parámetros inseguros comunes en hardening.

## Ejecutar
```bash
make run
make test
```
