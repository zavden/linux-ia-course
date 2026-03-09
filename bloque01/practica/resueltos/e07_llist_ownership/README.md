# E07 — Lista genérica con ownership configurable

## Objetivo
Aprender a liberar datos dinámicos de forma correcta usando callback destructor.

## Qué hace
- Inserta `char*` reservados dinámicamente.
- Usa `destroy_ex(list, free_fn)` para liberar nodos + datos.
- Evita leaks sin acoplar la lista a un tipo concreto.

## Ejecutar
```bash
make run
make test
```
