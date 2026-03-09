# E08 — reactor con poll sobre múltiples clientes

## Objetivo
Implementar patrón reactor simple: esperar eventos y despachar por FD listo.

## Qué hace
- Simula 3 clientes con `socketpair`.
- El reactor lee, transforma a mayúsculas y responde.
- Verifica que todos los clientes reciban su respuesta.

## Ejecutar
```bash
make run
make test
```
