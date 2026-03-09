# EJERCICIOS.md — Bloque 03 (Índice)

Este archivo es solo mapa de ejercicios.
No incluye soluciones embebidas.

Práctica completa en `bloque03/practica/`.

---

## Resueltos (10)

## E01 — fork + wait básico
- Objetivo: crear hijos y recolectarlos correctamente.
- Qué hace: lanza 3 hijos con códigos distintos y el padre los recoge.
- Ruta: `bloque03/practica/resueltos/e01_fork_wait_basico`

## E02 — waitpid no bloqueante
- Objetivo: usar `WNOHANG` sin congelar el padre.
- Qué hace: polling no bloqueante hasta recolectar al hijo.
- Ruta: `bloque03/practica/resueltos/e02_waitpid_nohang`

## E03 — execvp básico
- Objetivo: reemplazar imagen de proceso hijo.
- Qué hace: ejecuta comando externo con `fork+execvp+waitpid`.
- Ruta: `bloque03/practica/resueltos/e03_execvp_basico`

## E04 — redirección con dup2
- Objetivo: redirigir `stdout` hacia archivo.
- Qué hace: hijo aplica `dup2` y ejecuta comando; padre espera.
- Ruta: `bloque03/practica/resueltos/e04_dup2_redireccion`

## E05 — pipe unidireccional
- Objetivo: IPC básico entre padre e hijo.
- Qué hace: padre envía mensaje, hijo lo transforma y responde por stdout.
- Ruta: `bloque03/practica/resueltos/e05_pipe_unidireccional`

## E06 — pipes bidireccionales
- Objetivo: comunicación de ida y vuelta.
- Qué hace: ping-pong con dos pipes (`down` y `up`).
- Ruta: `bloque03/practica/resueltos/e06_pipe_bidireccional`

## E07 — sigaction básico
- Objetivo: manejar señales con flags asíncronas seguras.
- Qué hace: cuenta `SIGUSR1` y termina limpio con `SIGTERM`.
- Ruta: `bloque03/practica/resueltos/e07_sigaction_basico`

## E08 — sección crítica con sigprocmask
- Objetivo: bloquear señales temporalmente.
- Qué hace: demuestra señal pendiente y entrega al desbloquear.
- Ruta: `bloque03/practica/resueltos/e08_sigprocmask_critica`

## E09 — daemon básico
- Objetivo: desacoplar proceso de la terminal.
- Qué hace: aplica doble fork y escribe log periódico en `/tmp`.
- Ruta: `bloque03/practica/resueltos/e09_daemon_basico`

## E10 — minishell mínimo
- Objetivo: loop de shell con ejecución externa.
- Qué hace: lee líneas, ejecuta comandos y soporta `exit`.
- Ruta: `bloque03/practica/resueltos/e10_minishell_minimo`

---

## Complejos (3)

## C01 — shell con job control
- Objetivo: gestionar foreground/background con `SIGCHLD`.
- Qué debe hacer: jobs en `&`, reap non-blocking, estado de jobs.
- Ruta: `bloque03/practica/complejos/c01_shell_job_control`

## C02 — daemon de producción
- Objetivo: ciclo de vida robusto de servicio.
- Qué debe hacer: pidfile, señales, reload, shutdown y limpieza.
- Ruta: `bloque03/practica/complejos/c02_daemon_produccion`

## C03 — shell con pipes y redirecciones
- Objetivo: ejecución tipo shell real con `|`, `>`, `>>`, `<`.
- Qué debe hacer: parseo mínimo, `dup2`, `pipe`, manejo de errores.
- Ruta: `bloque03/practica/complejos/c03_shell_pipes_redirs`

---

## Notas

1. Cada ejercicio incluye `README.md`, `src/`, `tests/`, `Makefile`.
2. Los resueltos están muy comentados para estudio guiado.
3. Los complejos son plantillas de implementación, no soluciones finales.
