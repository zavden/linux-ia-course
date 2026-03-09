# E03 — parser SELinux/AppArmor

## Objetivo
Parsear reportes de estado de SELinux y AppArmor para construir diagnóstico unificado.

## Qué hace
- Lee texto estilo `sestatus` y `aa-status`.
- Extrae modo SELinux (`enforcing/permissive/disabled`).
- Cuenta perfiles AppArmor en enforce/complain.

## Ejecutar
```bash
make run
make test
```
