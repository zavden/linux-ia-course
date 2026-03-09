# EJERCICIOS.md — Bloque 02 (Índice)

Este archivo es solo mapa de ejercicios.
No incluye soluciones embebidas.

Práctica completa en `bloque02/practica/`.

---

## Resueltos (10)

## E01 — copia robusta con syscalls
- Objetivo: implementar copia binaria segura con `open/read/write/close`.
- Qué hace: copia por bloques y maneja escrituras parciales.
- Ruta: `bloque02/practica/resueltos/e01_copy_syscalls`

## E02 — write_all y escrituras parciales
- Objetivo: dominar el patrón `write_all`.
- Qué hace: fuerza parciales y garantiza escritura completa.
- Ruta: `bloque02/practica/resueltos/e02_write_all_parcial`

## E03 — stdio vs syscalls
- Objetivo: comparar enfoques de I/O byte-a-byte.
- Qué hace: mide tiempos de copia con syscalls y con stdio.
- Ruta: `bloque02/practica/resueltos/e03_stdio_vs_syscalls`

## E04 — umask y modo final
- Objetivo: entender permisos efectivos de creación.
- Qué hace: aplica `umask(0)` temporal y verifica modo resultante.
- Ruta: `bloque02/practica/resueltos/e04_umask_y_modo`

## E05 — chmod/fchmod seguro
- Objetivo: aplicar permisos sobre FD abierto.
- Qué hace: cambia permisos con `fchmod` y valida con `fstat`.
- Ruta: `bloque02/practica/resueltos/e05_chmod_seguro`

## E06 — stat vs lstat
- Objetivo: distinguir metadatos del target vs symlink.
- Qué hace: imprime tipo/inodo/enlaces usando ambas llamadas.
- Ruta: `bloque02/practica/resueltos/e06_stat_vs_lstat`

## E07 — listado de directorio
- Objetivo: recorrer entradas con `opendir/readdir`.
- Qué hace: muestra tipo, tamaño y nombre usando `lstat`.
- Ruta: `bloque02/practica/resueltos/e07_listar_directorio`

## E08 — hard link, symlink y unlink
- Objetivo: demostrar semántica real de enlaces.
- Qué hace: crea enlaces, borra nombre base y valida comportamiento.
- Ruta: `bloque02/practica/resueltos/e08_links_y_unlink`

## E09 — walk recursivo
- Objetivo: construir recorrido de árbol sin ciclos triviales.
- Qué hace: recursa directorios ignorando `.` y `..`.
- Ruta: `bloque02/practica/resueltos/e09_walk_recursivo`

## E10 — minifind básico
- Objetivo: integrar recorrido recursivo y filtros.
- Qué hace: filtra por `-name`, `-type`, `-size`.
- Ruta: `bloque02/practica/resueltos/e10_minifind_basico`

---

## Complejos (3)

## C01 — cp robusto de producción
- Objetivo: llevar copia de archivos a estándar de herramienta confiable.
- Qué debe hacer: manejo completo de parciales, errores y cleanup consistente.
- Ruta: `bloque02/practica/complejos/c01_cp_robusto`

## C02 — ls -l recursivo
- Objetivo: listar metadatos ricos en árbol completo.
- Qué debe hacer: permisos simbólicos, tamaños, tiempos y robustez ante errores.
- Ruta: `bloque02/practica/complejos/c02_ls_l_recursivo`

## C03 — minifind-plus
- Objetivo: extender minifind con filtros avanzados.
- Qué debe hacer: glob, depth, filtros combinados y exit status robusto.
- Ruta: `bloque02/practica/complejos/c03_minifind_plus`

---

## Notas

1. Cada ejercicio tiene `README.md`, `src/`, `tests/` y `Makefile`.
2. Los resueltos están altamente comentados para aprendizaje paso a paso.
3. Los complejos son plantillas guiadas, no solución final.
