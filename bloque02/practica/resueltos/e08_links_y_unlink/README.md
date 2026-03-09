# E08 — Hard link, symlink y unlink

## Objetivo
Demostrar experimentalmente la diferencia entre hard links y symlinks.

## Qué hace
- Crea archivo base con contenido.
- Crea hard link y symlink.
- Elimina nombre original (`unlink`).
- Verifica que hard link conserva datos y symlink queda roto.

## Ejecutar
```bash
make run
make test
```
