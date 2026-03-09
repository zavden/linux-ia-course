# EJERCICIOS.md — Bloque 01 (Índice)

Este archivo es solo un mapa de ejercicios.
No incluye soluciones completas embebidas.

El contenido práctico está en `bloque01/practica/`.

---

## Resueltos (10)

## E01 — getopt_long básico
- Objetivo: parsear opciones cortas/largas con validación robusta de enteros.
- Qué hace: procesa `-v`, `-n`, `-h`, muestra configuración final.
- Ruta: `bloque01/practica/resueltos/e01_getopt_long_basico`

## E02 — optind y posicionales
- Objetivo: separar opciones de argumentos posicionales usando `optind`.
- Qué hace: aplica transformación opcional a tokens posicionales y soporta `--`.
- Ruta: `bloque01/practica/resueltos/e02_optind_y_posicionales`

## E03 — safe_strcpy
- Objetivo: implementar copia segura estilo `strlcpy`.
- Qué hace: copia con límite, garantiza `\0`, reporta truncamiento.
- Ruta: `bloque01/practica/resueltos/e03_safe_strcpy`

## E04 — safe_strcat
- Objetivo: concatenar strings sin overflow.
- Qué hace: respeta capacidad de destino y detecta truncamiento por retorno.
- Ruta: `bloque01/practica/resueltos/e04_safe_strcat`

## E05 — safe_snprintf
- Objetivo: encapsular formato variádico seguro.
- Qué hace: wrapper de `vsnprintf` con detección de truncamiento.
- Ruta: `bloque01/practica/resueltos/e05_safe_snprintf`

## E06 — lista genérica básica
- Objetivo: construir `create/push/pop/destroy` con `void *`.
- Qué hace: manipula una lista enlazada simple y libera nodos correctamente.
- Ruta: `bloque01/practica/resueltos/e06_llist_basica`

## E07 — lista con ownership
- Objetivo: resolver ownership de memoria en estructuras genéricas.
- Qué hace: usa destructor callback para liberar nodos y datos.
- Ruta: `bloque01/practica/resueltos/e07_llist_ownership`

## E08 — goto cleanup
- Objetivo: unificar liberación de recursos ante múltiples errores.
- Qué hace: demuestra patrón `goto cleanup` con `malloc` + `fopen`.
- Ruta: `bloque01/practica/resueltos/e08_goto_cleanup`

## E09 — macro CHECK_SYS
- Objetivo: reducir boilerplate en verificación de syscalls.
- Qué hace: imprime archivo/línea/expresión cuando una llamada falla.
- Ruta: `bloque01/practica/resueltos/e09_check_sys`

## E10 — benchmark memcpy
- Objetivo: medir implementación ingenua vs libc.
- Qué hace: compara tiempos de `my_memcpy` y `memcpy` con `clock_gettime`.
- Ruta: `bloque01/practica/resueltos/e10_benchmark_memcpy`

---

## Complejos (3)

## C01 — argflow
- Objetivo: CLI robusta de nivel utilitario real.
- Qué debe hacer: opciones mixtas, validación estricta, posicionales, códigos de salida consistentes.
- Ruta: `bloque01/practica/complejos/c01_argflow`

## C02 — lista genérica extendida
- Objetivo: API completa reusable con invariantes y ownership configurable.
- Qué debe hacer: `push_front/back`, `find`, `foreach`, `destroy_ex`, sin leaks.
- Ruta: `bloque01/practica/complejos/c02_llist_extendida`

## C03 — minicat-lite
- Objetivo: clon reducido de `cat` con semántica realista de errores.
- Qué debe hacer: archivos + stdin, `-n`, `-b`, continuar tras fallos y exit status final correcto.
- Ruta: `bloque01/practica/complejos/c03_minicat_lite`

---

## Notas

1. Cada ejercicio tiene `README.md`, `src/` y `tests/`.
2. El código en `resueltos/` está comentado de forma pedagógica.
3. El código en `complejos/` es plantilla guiada, no solución final.
