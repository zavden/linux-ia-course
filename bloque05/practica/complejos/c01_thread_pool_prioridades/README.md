# C01 — thread pool con prioridades

## Objetivo
Diseñar un thread pool con cola de prioridades y apagado seguro.

## Qué debe hacer
- Cola protegida por mutex/condvar.
- Prioridad alta/media/baja con política de desempate FIFO.
- Señal de shutdown que drene cola y cierre workers sin fugas.

## Pistas
- Usa una estructura `task_t` con `priority`, `seq`, `fn`, `arg`.
- Implementa `pool_submit`, `pool_shutdown`, `pool_join`.
- Añade métricas: tareas completadas por prioridad y latencia promedio.
