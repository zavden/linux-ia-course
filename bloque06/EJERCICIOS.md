# EJERCICIOS.md — Bloque 06 (Índice)

Este archivo es solo mapa de ejercicios.
No incluye enunciados largos ni solución embebida.

Práctica completa en `bloque06/practica/`.

---

## Resueltos (10)

## E01 — socketpair básico
- Objetivo: entender comunicación full-duplex entre dos FDs conectados.
- Qué hace: envía `ping` y responde `pong` entre extremos locales.
- Ruta: `bloque06/practica/resueltos/e01_socketpair_basico`

## E02 — TCP loopback: bind/listen/accept
- Objetivo: dominar ciclo completo servidor-cliente TCP.
- Qué hace: servidor local acepta conexión y responde eco estructurado.
- Ruta: `bloque06/practica/resueltos/e02_tcp_loopback_bind_listen_accept`

## E03 — non-blocking con fcntl
- Objetivo: activar `O_NONBLOCK` y tratar `EAGAIN/EWOULDBLOCK`.
- Qué hace: demuestra lectura no bloqueante en pipe con y sin datos.
- Ruta: `bloque06/practica/resueltos/e03_nonblocking_fcntl_pipe`

## E04 — multiplexación con select
- Objetivo: esperar múltiples descriptores en un solo punto de bloqueo.
- Qué hace: detecta cuál pipe está listo para lectura con `select`.
- Ruta: `bloque06/practica/resueltos/e04_select_multiplexacion`

## E05 — multiplexación con poll
- Objetivo: usar `pollfd` y `revents` para dispatch por FD.
- Qué hace: valida eventos `POLLIN` sobre dos fuentes.
- Ruta: `bloque06/practica/resueltos/e05_poll_multiplexacion`

## E06 — framing de líneas
- Objetivo: extraer mensajes completos desde stream fragmentado.
- Qué hace: parser incremental que conserva remanente parcial.
- Ruta: `bloque06/practica/resueltos/e06_framing_lineas`

## E07 — parser de protocolo key/value
- Objetivo: parsear comandos textuales de cache.
- Qué hace: implementa `SET/GET/DEL` con respuestas `OK/VALUE/NULL/ERR`.
- Ruta: `bloque06/practica/resueltos/e07_parser_protocolo_kv`

## E08 — reactor con poll sobre múltiples clientes
- Objetivo: construir dispatcher single-thread por eventos de lectura.
- Qué hace: atiende varios clientes simulados y responde transformación.
- Ruta: `bloque06/practica/resueltos/e08_reactor_poll_socketpairs`

## E09 — backpressure en socket no bloqueante
- Objetivo: detectar saturación de envío y reintentar tras drenaje.
- Qué hace: escribe hasta `EAGAIN`, drena receptor y continúa.
- Ruta: `bloque06/practica/resueltos/e09_backpressure_no_bloqueante`

## E10 — minicache con event loop y poll
- Objetivo: integrar reactor, parser incremental y almacenamiento compartido.
- Qué hace: procesa scripts de clientes concurrentes con protocolo cache.
- Ruta: `bloque06/practica/resueltos/e10_minicache_eventloop_poll`

---

## Complejos (3)

## C01 — reactor epoll/poll con abstracción
- Objetivo: diseñar backend dual portable y eficiente.
- Qué debe hacer: API unificada de registro/espera/eventos para ambos backends.
- Ruta: `bloque06/practica/complejos/c01_reactor_epoll_poll_abstraccion`

## C02 — minicurl no bloqueante
- Objetivo: cliente HTTP robusto con timeout y connect asíncrono.
- Qué debe hacer: DNS, connect no bloqueante, request/response incremental.
- Ruta: `bloque06/practica/complejos/c02_minicurl_no_bloqueante`

## C03 — minicache con LRU y persistencia
- Objetivo: evolucionar cache en memoria con política de evicción y snapshot.
- Qué debe hacer: LRU, comandos de persistencia y métricas internas.
- Ruta: `bloque06/practica/complejos/c03_minicache_lru_persistencia`

---

## Notas

1. Cada ejercicio incluye `README.md`, `src/`, `tests/`, `Makefile`.
2. Los resueltos están comentados en detalle para estudio paso a paso.
3. Los complejos son plantillas guiadas para implementación propia.
