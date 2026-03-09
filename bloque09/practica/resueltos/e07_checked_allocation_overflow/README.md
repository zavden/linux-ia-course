# E07 - asignacion con chequeo de overflow

## Objetivo
Evitar integer overflow al calcular tamanos de memoria antes de reservar buffers.

## Que hace
- Parsea dos tamanos (`n` y `elem_size`) de forma estricta.
- Usa multiplicacion protegida para detectar overflow.
- Reserva memoria solo si el calculo es seguro.

## Ejecutar
```bash
make run
make test
```
