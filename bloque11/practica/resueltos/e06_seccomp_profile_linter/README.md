# E06 - linter básico de perfil seccomp

## Objetivo
Validar reglas mínimas de un perfil seccomp para reducir superficie de syscalls.

## Qué hace
- Revisa `defaultAction` del perfil JSON.
- Verifica presencia de syscalls base requeridas.
- Detecta syscalls peligrosas permitidas.

## Ejecutar
```bash
make run
make test
```
