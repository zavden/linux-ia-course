# C03 — benchmark de contención de locks

## Objetivo
Comparar impacto de sincronización (`mutex`, `rwlock`, sharding) bajo carga concurrente.

## Qué debe hacer
- Ejecutar escenarios controlados con distintos números de hilos.
- Medir throughput (ops/s) y latencia promedio.
- Reportar escalabilidad por estrategia.

## Pistas
- Usa `clock_gettime(CLOCK_MONOTONIC, ...)` para medir.
- Escenarios sugeridos: 90/10 lectura/escritura y 50/50.
- Exporta resultados en CSV para graficar.
