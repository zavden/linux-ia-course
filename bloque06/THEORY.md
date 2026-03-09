# THEORY.md — Bloque 06: Redes y Multiplexación en C

Este bloque enseña cómo construir servicios de red eficientes sin crear un hilo por conexión.
La meta práctica es dominar **I/O no bloqueante + multiplexación + protocolo robusto**.

---

## 1. Fundamentos de sockets TCP

### 1.1 Flujo servidor clásico
Ciclo básico en servidor TCP:
1. `socket()`
2. `setsockopt(... SO_REUSEADDR ...)`
3. `bind()`
4. `listen()`
5. `accept()`
6. `read()/write()` sobre el socket cliente
7. `close()`

Puntos finos:
- `socket()` crea endpoint local del proceso, no conexión remota.
- `bind()` asocia IP/puerto local.
- `listen()` habilita cola de conexiones pendientes.
- `accept()` devuelve **nuevo FD** por cliente; el FD de escucha sigue activo.

### 1.2 Endianness de red
- Puerto en `sockaddr_in` va con `htons`.
- IPv4 en estructuras usa formato de red (big-endian).
- Para logging y parseo: `inet_ntop` / `inet_pton`.

### 1.3 `SO_REUSEADDR`
Antes de `bind`, conviene:
```c
int yes = 1;
setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
```
Evita errores de reinicio rápido por sockets en `TIME_WAIT`.

---

## 2. Bloqueante vs no bloqueante

### 2.1 Problema del modo bloqueante
En modo bloqueante, una llamada lenta (`read`, `accept`, `connect`) puede detener todo el event loop.
Eso destruye escalabilidad en servidores single-thread.

### 2.2 Activar `O_NONBLOCK`
```c
int flags = fcntl(fd, F_GETFL, 0);
fcntl(fd, F_SETFL, flags | O_NONBLOCK);
```

### 2.3 Interpretar errores esperados
Con no bloqueante son normales:
- `EAGAIN`
- `EWOULDBLOCK`

No son “fallo fatal”; significan “ahora no hay progreso posible, vuelve luego”.

### 2.4 `connect` no bloqueante
- `connect` puede devolver `-1` con `EINPROGRESS`.
- Debes esperar writability (`poll/select/epoll`) y luego consultar `SO_ERROR`.

---

## 3. Multiplexación: `select`, `poll`, `epoll`

### 3.1 `select`
- Portable.
- Límite de `FD_SETSIZE`.
- Complejidad O(N) por escaneo completo de bitmap.

Útil para aprendizaje y pocos FDs.

### 3.2 `poll`
- Más cómodo que `select` (array de `struct pollfd`).
- Sin límite duro de 1024 por API, pero sigue O(N).
- Muy útil para reactores medianos y portabilidad POSIX amplia.

### 3.3 `epoll` (Linux)
- API específica de Linux.
- Mejor rendimiento con muchos FDs activos/inactivos.
- Operaciones clave:
  - `epoll_create1`
  - `epoll_ctl(ADD/MOD/DEL)`
  - `epoll_wait`

### 3.4 Level-triggered vs edge-triggered
- **Level-triggered (LT)**: mientras haya datos pendientes, seguirá notificando.
- **Edge-triggered (ET)**: notifica transición; debes drenar hasta `EAGAIN` o pierdes eventos.

Recomendación didáctica inicial: LT por simplicidad y menor riesgo de bugs.

---

## 4. Reactor pattern

Arquitectura típica de servidor single-thread:
1. FDs registrados en selector (`poll/epoll`)
2. `wait` bloqueante eficiente
3. Dispatcher por evento:
   - nuevo cliente (`accept`)
   - lectura (`read`)
   - escritura pendiente (`write`)
   - cierre/error (`close` + limpieza estado)

Clave de diseño: cada conexión necesita estado propio (buffers de entrada/salida, parser, flags).

---

## 5. Framing y protocolos

TCP es stream de bytes, no “mensajes”.
No existe garantía de que un `write` remoto coincida con un `read` local.

Necesitas framing explícito, por ejemplo:
- delimitador de línea (`\n`)
- longitud prefijada (`len + payload`)
- formato binario con cabecera

### 5.1 Parser incremental
Regla de oro:
- acumular bytes en buffer por conexión
- extraer solo mensajes completos
- conservar remanente incompleto para siguiente iteración

### 5.2 Manejo de entrada inválida
- validar tamaño máximo por línea/mensaje
- responder error de protocolo
- cerrar conexión si se viola contrato repetidamente

---

## 6. Backpressure y control de escritura

### 6.1 Qué pasa cuando el peer no lee
Incluso si tu socket es no bloqueante, `write` puede:
- escribir parcial
- devolver `EAGAIN`

### 6.2 Estrategia correcta
- mantener cola de salida por conexión
- en `write` parcial, conservar resto pendiente
- pedir evento de escritura cuando haya datos pendientes
- desactivar evento de escritura al vaciar cola

Sin esto, perderás datos o bloquearás.

---

## 7. Cierre ordenado y ciclo de vida de FD

### 7.1 EOF y half-close
- `read == 0` => peer cerró su lado de escritura.
- Puedes aún tener datos pendientes por enviar según protocolo.

### 7.2 Reglas de higiene
- `close(fd)` exactamente una vez.
- remover FD del reactor y limpiar su estado asociado.
- evitar usar FD después de cerrarlo (use-after-close).

### 7.3 Señales e interrupciones
Syscalls bloqueantes pueden fallar con `EINTR`.
Debes decidir reintento o salida controlada.

---

## 8. Errores frecuentes en servidores de aprendizaje

1. Suponer que `read` devuelve una línea completa.
2. Ignorar escrituras parciales.
3. Tratar `EAGAIN` como error fatal.
4. No limpiar estado al cerrar FD.
5. Usar buffers globales compartidos entre clientes sin indexar por conexión.
6. Basar protocolo en timing (`sleep`) en lugar de framing.
7. No limitar tamaño de entrada y permitir overflow lógico.

---

## 9. Mini-checklist de robustez

Antes de dar un servidor por “bueno”:
1. ¿Todos los sockets relevantes están en no bloqueante?
2. ¿Manejas `EAGAIN/EWOULDBLOCK/EINTR`?
3. ¿Soportas lectura/escritura parcial?
4. ¿Cada conexión tiene buffer y parser propio?
5. ¿Tu event loop no hace busy-wait innecesario?
6. ¿Hay límites de memoria por conexión?
7. ¿Limpias estado al desconectar?

---

## 10. Mapa de práctica del bloque

### Resueltos
- `e01_socketpair_basico`: comunicación bidireccional local.
- `e02_tcp_loopback_bind_listen_accept`: ciclo completo TCP servidor/cliente.
- `e03_nonblocking_fcntl_pipe`: activación de no bloqueante y manejo `EAGAIN`.
- `e04_select_multiplexacion`: espera múltiple con `select`.
- `e05_poll_multiplexacion`: espera múltiple con `poll`.
- `e06_framing_lineas`: parser incremental por líneas.
- `e07_parser_protocolo_kv`: protocolo textual `SET/GET/DEL`.
- `e08_reactor_poll_socketpairs`: reactor simple con dispatch por FD.
- `e09_backpressure_no_bloqueante`: detección de buffer lleno y reintento.
- `e10_minicache_eventloop_poll`: integración completa de loop + parser + store.

### Complejos
- `c01_reactor_epoll_poll_abstraccion`: backend dual Linux/portable.
- `c02_minicurl_no_bloqueante`: cliente HTTP no bloqueante con timeout.
- `c03_minicache_lru_persistencia`: cache con política LRU y snapshot.

---

## 11. Conexión con proyecto

Para un `minicache`/`miniserver` real necesitas combinar:
- reactor robusto
- parser incremental
- almacenamiento consistente
- respuesta correcta ante desconexiones y errores de red

Ese es exactamente el puente entre ejercicios del bloque y el proyecto final de red.
