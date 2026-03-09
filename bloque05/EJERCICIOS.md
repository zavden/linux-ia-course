# EJERCICIOS.md — Bloque 05 (Índice)

Este archivo es solo mapa de ejercicios.
No incluye enunciados largos ni soluciones embebidas.

Práctica completa en `bloque05/practica/`.

---

## Resueltos (10)

## E01 — pthread create/join básico
- Objetivo: lanzar hilos, repartir trabajo y consolidar resultados.
- Qué hace: divide arreglo en bloques y suma parcial por hilo.
- Ruta: `bloque05/practica/resueltos/e01_pthread_create_join`

## E02 — argumentos por struct y retorno en join
- Objetivo: pasar múltiples parámetros y recuperar resultado de cada hilo.
- Qué hace: suma cuadrados por rangos y agrega retornos por `pthread_join`.
- Ruta: `bloque05/practica/resueltos/e02_args_struct_y_join`

## E03 — race condition demostrable
- Objetivo: visualizar pérdida de updates sin sincronización.
- Qué hace: fuerza intercalado lectura/escritura sobre contador compartido.
- Ruta: `bloque05/practica/resueltos/e03_race_condition_demo`

## E04 — mutex para contador seguro
- Objetivo: corregir sección crítica con exclusión mutua.
- Qué hace: protege `counter++` con `pthread_mutex_lock/unlock`.
- Ruta: `bloque05/practica/resueltos/e04_mutex_contador_seguro`

## E05 — condvar productor/consumidor
- Objetivo: esperar eventos sin busy-wait.
- Qué hace: coordina productor y consumidor con buffer acotado.
- Ruta: `bloque05/practica/resueltos/e05_condvar_productor_consumidor`

## E06 — cola con condvar y cierre limpio
- Objetivo: resolver multi-productor/multi-consumidor sin deadlocks al terminar.
- Qué hace: usa cola circular, condvars y protocolo de cierre por productores vivos.
- Ruta: `bloque05/practica/resueltos/e06_condvar_queue_cierre`

## E07 — rwlock para caché compartida
- Objetivo: permitir lecturas concurrentes y escrituras exclusivas.
- Qué hace: combina lectores/escritores y valida estado final de caché.
- Ruta: `bloque05/practica/resueltos/e07_rwlock_cache_basica`

## E08 — semáforo como rate limiter
- Objetivo: limitar concurrencia máxima con contador de tokens.
- Qué hace: controla 10 hilos permitiendo solo 3 simultáneos en sección crítica.
- Ruta: `bloque05/practica/resueltos/e08_semaforo_rate_limiter`

## E09 — thread pool mínimo
- Objetivo: desacoplar producción de tareas y ejecución en workers persistentes.
- Qué hace: cola thread-safe + 4 workers que procesan 20 tareas.
- Ruta: `bloque05/practica/resueltos/e09_thread_pool_minimo`

## E10 — pipeline multietapa
- Objetivo: encadenar transformaciones concurrentes por etapas.
- Qué hace: dos etapas conectadas por colas con sentinel de finalización.
- Ruta: `bloque05/practica/resueltos/e10_pipeline_multietapa`

---

## Complejos (3)

## C01 — thread pool con prioridades
- Objetivo: diseñar scheduler concurrente con fairness y shutdown seguro.
- Qué debe hacer: colas priorizadas, workers persistentes y métricas por clase.
- Ruta: `bloque05/practica/complejos/c01_thread_pool_prioridades`

## C02 — mini servidor HTTP con pool
- Objetivo: integrar sockets + cola + workers para servir requests concurrentes.
- Qué debe hacer: `accept` en productor, parsing HTTP mínimo y respuesta 200/404.
- Ruta: `bloque05/practica/complejos/c02_miniserver_pool_http`

## C03 — benchmark de contención de locks
- Objetivo: medir impacto real de distintas estrategias de sincronización.
- Qué debe hacer: comparar mutex/rwlock/sharding con métricas reproducibles.
- Ruta: `bloque05/practica/complejos/c03_benchmark_lock_contention`

---

## Notas

1. Cada ejercicio incluye `README.md`, `src/`, `tests/`, `Makefile`.
2. Los resueltos están fuertemente comentados para estudio paso a paso.
3. Los complejos son plantillas guiadas: la implementación final te corresponde a ti.
