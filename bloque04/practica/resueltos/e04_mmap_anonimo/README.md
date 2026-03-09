# E04 — mmap anónimo

## Objetivo
Reservar memoria anónima con `mmap` sin usar `malloc`.

## Qué hace
- Reserva región anónima RW.
- Escribe patrón de bytes.
- Libera con `munmap`.

## Ejecutar
```bash
make run
make test
```
