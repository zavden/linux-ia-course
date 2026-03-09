# E01 — fork + wait básico

## Objetivo
Crear varios hijos, recolectarlos y verificar códigos de salida.

## Qué hace
- Crea 3 hijos con `fork()`.
- Cada hijo termina con `exit(10+i)`.
- El padre usa `waitpid` y muestra estado.

## Ejecutar
```bash
make run
make test
```
