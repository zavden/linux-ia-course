# E05 — shared memory padre/hijo

## Objetivo
Compartir un buffer entre padre e hijo usando `shm_open` + `mmap`.

## Qué hace
- Crea objeto POSIX SHM.
- Padre escribe texto.
- Hijo lo lee y responde en la misma región.
- Padre imprime respuesta tras `waitpid`.

## Ejecutar
```bash
make run
make test
```
