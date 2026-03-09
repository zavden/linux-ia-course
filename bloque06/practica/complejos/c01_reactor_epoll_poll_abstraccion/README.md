# C01 — reactor epoll/poll con abstracción

## Objetivo
Diseñar un reactor portable que use `epoll` en Linux y `poll` como fallback.

## Qué debe hacer
- API única: `reactor_add`, `reactor_mod`, `reactor_del`, `reactor_wait`.
- Soportar `READ`, `WRITE`, `ERROR` como eventos abstractos.
- Manejar crecimiento dinámico de FDs registrados.

## Pistas
- Usa `#ifdef __linux__` para backend `epoll`.
- En fallback `poll`, reconstruye arreglo activo eficientemente.
- Añade pruebas con `socketpair` para validar ambos caminos.
