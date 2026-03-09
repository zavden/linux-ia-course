# EJERCICIOS.md — Bloque 04 (Índice)

Este archivo es solo mapa de ejercicios.
No incluye soluciones embebidas.

Práctica completa en `bloque04/practica/`.

---

## Resueltos (10)

## E01 — DynArray básico
- Objetivo: construir vector dinámico con crecimiento amortizado.
- Qué hace: inserta elementos y duplica capacidad con `realloc`.
- Ruta: `bloque04/practica/resueltos/e01_dynarray_basico`

## E02 — realloc seguro
- Objetivo: evitar pérdida de puntero al realocar.
- Qué hace: aplica patrón temporal seguro y preserva datos.
- Ruta: `bloque04/practica/resueltos/e02_realloc_seguro`

## E03 — mmap de archivo básico
- Objetivo: editar archivo vía memoria mapeada.
- Qué hace: mapea, modifica primer byte y sincroniza con `msync`.
- Ruta: `bloque04/practica/resueltos/e03_mmap_archivo_basico`

## E04 — mmap anónimo
- Objetivo: reservar memoria fuera de `malloc` usando mapeo anónimo.
- Qué hace: crea región anónima, escribe patrón y libera con `munmap`.
- Ruta: `bloque04/practica/resueltos/e04_mmap_anonimo`

## E05 — shared memory padre/hijo
- Objetivo: compartir buffer en RAM entre procesos.
- Qué hace: usa `shm_open + ftruncate + mmap + fork` y cleanup completo.
- Ruta: `bloque04/practica/resueltos/e05_shm_padre_hijo`

## E06 — RLIMIT_NOFILE
- Objetivo: observar y ajustar límite de file descriptors.
- Qué hace: imprime soft/hard, reduce soft y verifica cambio.
- Ruta: `bloque04/practica/resueltos/e06_rlimit_nofile`

## E07 — RLIMIT_AS (o fallback)
- Objetivo: experimentar límites de memoria por proceso.
- Qué hace: ajusta límite de memoria y reserva bloques hasta fallo/umbral.
- Ruta: `bloque04/practica/resueltos/e07_rlimit_as`

## E08 — pool allocator lineal
- Objetivo: implementar asignación O(1) por desplazamiento.
- Qué hace: reserva bloque grande y reparte sub-bloques sin free individual.
- Ruta: `bloque04/practica/resueltos/e08_pool_allocator`

## E09 — parser de meminfo
- Objetivo: extraer métricas de memoria desde formato `/proc/meminfo`.
- Qué hace: parsea campos clave y calcula memoria usada aproximada.
- Ruta: `bloque04/practica/resueltos/e09_proc_meminfo_parser`

## E10 — minitop mini
- Objetivo: integrar memoria global y listado de PIDs.
- Qué hace: lee meminfo + directorio tipo `/proc` y muestra tabla simple.
- Ruta: `bloque04/practica/resueltos/e10_minitop_mini`

---

## Complejos (3)

## C01 — allocator segmentado
- Objetivo: reducir fragmentación con clases de tamaño.
- Qué debe hacer: free-lists por clase + fallback para bloques grandes.
- Ruta: `bloque04/practica/complejos/c01_allocator_segmentado`

## C02 — editor binario con mmap
- Objetivo: editar bytes por offset de forma segura y persistente.
- Qué debe hacer: validación fuerte, mapeo compartido, sync y cleanup.
- Ruta: `bloque04/practica/complejos/c02_mmap_editor_binario`

## C03 — minitop interactivo
- Objetivo: monitor en refresco continuo con ordenamiento por RSS.
- Qué debe hacer: parseo de `/proc`, orden, top N y tolerancia a errores.
- Ruta: `bloque04/practica/complejos/c03_minitop_interactivo`

---

## Notas

1. Cada ejercicio incluye `README.md`, `src/`, `tests/`, `Makefile`.
2. Los resueltos están comentados para estudio guiado paso a paso.
3. Los complejos son plantillas de implementación, no solución final.
