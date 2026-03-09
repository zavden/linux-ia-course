# C02 — mini servidor HTTP con pool

## Objetivo
Construir un servidor TCP/HTTP mínimo atendido por thread pool fijo.

## Qué debe hacer
- `socket/bind/listen/accept` en hilo principal.
- Cola de `client_fd` para workers.
- Parseo de primera línea HTTP (`GET /ruta HTTP/1.1`).
- Respuesta `200` o `404` con cabeceras válidas.

## Pistas
- Reusa el patrón de cola bloqueante de E09.
- Cierra siempre `client_fd` en worker, incluso en error.
- Soporta apagado por señal (`SIGINT`) sin dejar sockets abiertos.
