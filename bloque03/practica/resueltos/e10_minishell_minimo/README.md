# E10 — minishell mínimo

## Objetivo
Construir un mini loop de shell con ejecución externa y builtin `exit`.

## Qué hace
- Lee líneas de `stdin`.
- Ejecuta comando externo por línea (`fork+execvp+waitpid`).
- Termina con comando `exit`.

## Ejecutar
```bash
make run
make test
```
