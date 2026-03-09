# EJERCICIOS.md — Bloque 07 (Índice)

Este archivo es solo mapa de ejercicios.
No incluye enunciado completo ni solución embebida.

Práctica completa en `bloque07/practica/`.

---

## Resueltos (10)

## E01 — assert y fail-fast
- Objetivo: validar invariantes y fallar temprano en estados imposibles.
- Qué hace: usa `assert` + validación explícita de errores.
- Ruta: `bloque07/practica/resueltos/e01_assert_failfast`

## E02 — macros de trazas por nivel
- Objetivo: controlar verbosidad de logs por nivel configurable.
- Qué hace: emite logs `ERR/INFO/DBG` y cuenta mensajes publicados.
- Ruta: `bloque07/practica/resueltos/e02_trace_macros`

## E03 — contexto de errores con errno
- Objetivo: reportar errores de sistema con información accionable.
- Qué hace: provoca fallo de `fopen`, traduce `errno` y muestra contexto.
- Ruta: `bloque07/practica/resueltos/e03_errno_contexto`

## E04 — proyecto multifile con Make
- Objetivo: separar lógica en módulos y compilar incrementalmente.
- Qué hace: integra módulos `calc` y `fmt` desde `main`.
- Ruta: `bloque07/practica/resueltos/e04_multifile_make_basico`

## E05 — mini test harness en C
- Objetivo: crear runner básico de pruebas unitarias sin frameworks externos.
- Qué hace: define `EXPECT_EQ` y reporta `passed/failed`.
- Ruta: `bloque07/practica/resueltos/e05_mini_unittest_harness`

## E06 — feature flags por preprocesador
- Objetivo: alternar implementaciones con `#ifdef` de forma segura.
- Qué hace: compara ruta normal vs ruta rápida para misma función.
- Ruta: `bloque07/practica/resueltos/e06_preprocessor_feature_flags`

## E07 — librería estática local (`.a`)
- Objetivo: entender flujo de `obj` -> `ar` -> binario final.
- Qué hace: construye `libtextstats.a` y la enlaza con `main`.
- Ruta: `bloque07/practica/resueltos/e07_static_library_local`

## E08 — dispatch table con punteros a función
- Objetivo: reemplazar cadenas de `if/else` por tabla extensible.
- Qué hace: parsea comandos aritméticos y ejecuta función asociada.
- Ruta: `bloque07/practica/resueltos/e08_dispatch_table_comandos`

## E09 — benchmark básico con clock_gettime
- Objetivo: medir tiempo de ejecución en nanosegundos.
- Qué hace: perfila una carga sintética con reloj monotónico.
- Ruta: `bloque07/practica/resueltos/e09_clock_benchmark_basico`

## E10 — mini profiler por scopes
- Objetivo: instrumentar tiempos por etapa funcional.
- Qué hace: mide dos workloads y reporta métricas por scope + total.
- Ruta: `bloque07/practica/resueltos/e10_mini_profiler_scopes`

---

## Complejos (3)

## C01 — calculadora CLI depurable
- Objetivo: construir CLI modular con estrategia de depuración profesional.
- Qué debe hacer: parser robusto, errores contextuales, modo traza y tests.
- Ruta: `bloque07/practica/complejos/c01_debuggable_cli_calculadora`

## C02 — build system estática + compartida
- Objetivo: diseñar pipeline de build para múltiples artefactos.
- Qué debe hacer: generar `.a` y `.so/.dylib`, linkear y validar símbolos.
- Ruta: `bloque07/practica/complejos/c02_build_system_static_shared`

## C03 — laboratorio guiado de bugs de memoria
- Objetivo: practicar detección y corrección de bugs de memoria reproducibles.
- Qué debe hacer: escenarios activables por CLI y guía de diagnóstico.
- Ruta: `bloque07/practica/complejos/c03_memory_bug_lab_guiado`

---

## Notas

1. Cada ejercicio incluye `README.md`, `src/`, `tests/`, `Makefile`.
2. Los resueltos están comentados para aprendizaje paso a paso.
3. Los complejos son plantillas guiadas, no solución final.
