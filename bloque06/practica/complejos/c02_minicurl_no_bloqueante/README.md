# C02 — minicurl no bloqueante

## Objetivo
Construir un cliente HTTP mínimo con connect no bloqueante y timeout controlado.

## Qué debe hacer
- Resolver host/puerto con `getaddrinfo`.
- `connect` no bloqueante + espera con `poll`/`select`.
- Enviar request `GET` y leer respuesta incremental.
- Imprimir status line y tamaño de body.

## Pistas
- Verifica `SO_ERROR` tras `poll` en socket conectante.
- Maneja `EINTR`, `EAGAIN`, `EWOULDBLOCK` en bucle robusto.
- Añade opción de timeout por CLI.
