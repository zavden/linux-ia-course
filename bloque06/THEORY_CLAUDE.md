# THEORY_CLAUDE.md — Bloque 06: Redes y Multiplexación en C

> Este bloque te enseña a construir servidores de red reales en C. No el servidor de juguete que acepta un cliente y muere — sino un servidor que maneja cientos de conexiones simultáneas en un solo hilo, sin perder datos, sin bloquear, y sin crear un thread por cliente. La clave es el **reactor pattern**: I/O no bloqueante + multiplexación de eventos.

---

## Mapa del Bloque

```
Tema 1: Sockets TCP        →  socket/bind/listen/accept, el flujo completo
Tema 2: Bloq. vs no-bloq.  →  O_NONBLOCK, EAGAIN, EINPROGRESS
Tema 3: Multiplexación     →  select, poll, epoll — cuándo usar cada uno
Tema 4: Reactor pattern    →  Event loop single-thread con dispatch por FD
Tema 5: Framing y parsing  →  TCP es un stream, no mensajes
Tema 6: Backpressure       →  Qué hacer cuando el peer no lee
Tema 7: Cierre y limpieza  →  EOF, half-close, ciclo de vida de conexiones
```

---

## Tema 1 — Sockets TCP: el flujo completo

### El ciclo de vida de un servidor TCP

```
Servidor                                  Cliente
────────                                  ───────

socket()     → crea endpoint
setsockopt() → SO_REUSEADDR
bind()       → asocia IP:puerto
listen()     → habilita cola de espera
                                          socket()
                                          connect() ──────────────────▶ SYN
accept()  ◀──────────────────────────────── SYN+ACK ◀─── three-way
  │           ──────────────────────────────▶ ACK         handshake
  │
  ▼
client_fd    → nuevo FD para ESTE cliente
               (listen_fd sigue aceptando otros)
  │
read/write ◀────────────────────────────────▶ read/write
  │
close()      → cierra conexión
```

### Código del servidor mínimo

```c
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int create_server(uint16_t port) {
    // 1. Crear socket
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1) { perror("socket"); return -1; }

    // 2. Reutilizar puerto inmediatamente tras reinicio
    int yes = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    // 3. Bind: asociar IP y puerto
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port   = htons(port),         // host-to-network byte order
        .sin_addr   = { htonl(INADDR_ANY) } // escuchar en todas las interfaces
    };
    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind"); close(sfd); return -1;
    }

    // 4. Listen: habilitar cola de conexiones pendientes
    if (listen(sfd, 128) == -1) {
        perror("listen"); close(sfd); return -1;
    }

    return sfd;  // listo para accept()
}
```

### Aceptar conexiones

```c
struct sockaddr_in client_addr;
socklen_t addr_len = sizeof(client_addr);

int cfd = accept(sfd, (struct sockaddr *)&client_addr, &addr_len);
if (cfd == -1) { perror("accept"); /* manejar */ }

// Obtener IP del cliente para logging:
char ip_str[INET_ADDRSTRLEN];
inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
printf("Nuevo cliente: %s:%d\n", ip_str, ntohs(client_addr.sin_port));
```

> [!IMPORTANT]
> **`accept` devuelve un NUEVO FD** — el del cliente. El socket de escucha (`sfd`) sigue activo para aceptar más clientes. Cada cliente tiene su propio FD independiente.

### Endianness: por qué `htons`/`htonl`

Las redes usan **big-endian** (byte más significativo primero). Tu CPU probablemente usa **little-endian**. Las funciones de conversión:

| Función | Dirección | Uso |
|---------|-----------|-----|
| `htons` | Host → Network (16 bits) | Puertos: `htons(8080)` |
| `htonl` | Host → Network (32 bits) | Direcciones IPv4 |
| `ntohs` | Network → Host (16 bits) | Leer puerto recibido |
| `ntohl` | Network → Host (32 bits) | Leer dirección recibida |

### `SO_REUSEADDR`: por qué es obligatorio

Sin `SO_REUSEADDR`, si matas tu servidor y lo reinicias rápidamente, `bind` falla con `EADDRINUSE`. El TCP del kernel mantiene la conexión anterior en estado `TIME_WAIT` durante ~60 segundos. `SO_REUSEADDR` permite reutilizar el puerto mientras eso se resuelve.

---

## Tema 2 — Bloqueante vs no bloqueante

### El problema del modo bloqueante

```c
// Servidor single-thread bloqueante:
while (1) {
    int cfd = accept(sfd, NULL, NULL);     // ← BLOQUEA hasta que llegue cliente
    char buf[1024];
    ssize_t n = read(cfd, buf, sizeof(buf)); // ← BLOQUEA hasta que lleguen datos
    write(cfd, buf, n);                      // ← BLOQUEA si el buffer TCP está lleno
    close(cfd);
}
// Problema: mientras atiende un cliente, NO puede aceptar otros
```

### Activar `O_NONBLOCK`

```c
#include <fcntl.h>

void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) { perror("fcntl"); return; }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl");
    }
}
```

### Interpretar los errores "esperados"

Con sockets no bloqueantes, ciertos "errores" son normales:

| Errno | Significado | Qué hacer |
|-------|-------------|-----------|
| `EAGAIN` / `EWOULDBLOCK` | No hay datos ahora (read) o buffer lleno (write) | Volver al event loop, reintentar cuando el selector avise |
| `EINTR` | Syscall interrumpida por señal | Reintentar inmediatamente |
| `EINPROGRESS` | `connect()` está en curso | Esperar writability con poll/epoll, luego verificar `SO_ERROR` |
| `ECONNRESET` | El peer cerró violentamente (RST) | Cerrar el FD, limpiar estado |
| `EPIPE` | Escribir a un socket cuyo peer cerró lectura | Cerrar conexión |

> [!CAUTION]
> **`EAGAIN` no es un error** — es el mecanismo normal de I/O no bloqueante. Tratarlo como error fatal es el bug #1 en servidores de aprendizaje.

---

## Tema 3 — Multiplexación: esperar eventos en muchos FDs

### El problema: ¿cómo saber qué FD tiene datos?

Sin multiplexación, tu única opción es hacer `read` en cada FD secuencialmente, lo que bloquea o desperdicia CPU. La solución: pedirle al kernel que **te avise** cuándo algún FD está listo.

### `select`: el veterano (portable, limitado)

```c
fd_set read_fds;
FD_ZERO(&read_fds);
FD_SET(sfd, &read_fds);
FD_SET(client1, &read_fds);
FD_SET(client2, &read_fds);

int max_fd = /* el mayor de todos los FDs */;
int ready = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

if (FD_ISSET(sfd, &read_fds))     { /* nuevo cliente */ }
if (FD_ISSET(client1, &read_fds)) { /* datos del cliente 1 */ }
```

| ✅ Ventajas | ❌ Desventajas |
|------------|---------------|
| Disponible en todo POSIX | Límite duro de `FD_SETSIZE` (típicamente 1024) |
| Simple de entender | O(n): escanea todo el bitmap cada vez |
| | Debes reconstruir el `fd_set` tras cada llamada |

### `poll`: mejor ergonomía, sin límite duro

```c
struct pollfd fds[MAX_CLIENTS + 1];
int nfds = 0;

// Registrar socket de escucha
fds[nfds++] = (struct pollfd){ .fd = sfd, .events = POLLIN };

// Event loop:
while (1) {
    int ready = poll(fds, nfds, -1);  // -1 = bloquear indefinidamente
    if (ready == -1) {
        if (errno == EINTR) continue;
        perror("poll"); break;
    }

    for (int i = 0; i < nfds; i++) {
        if (fds[i].revents == 0) continue;  // nada en este FD

        if (fds[i].fd == sfd) {
            // Nuevo cliente
            int cfd = accept(sfd, NULL, NULL);
            set_nonblocking(cfd);
            fds[nfds++] = (struct pollfd){ .fd = cfd, .events = POLLIN };
        } else {
            // Datos de un cliente
            char buf[1024];
            ssize_t n = read(fds[i].fd, buf, sizeof(buf));
            if (n <= 0) {
                // EOF o error: cerrar
                close(fds[i].fd);
                fds[i] = fds[--nfds];  // compactar array
                i--;  // re-evaluar esta posición
            } else {
                // Procesar datos
                handle_data(fds[i].fd, buf, n);
            }
        }
    }
}
```

| ✅ Ventajas | ❌ Desventajas |
|------------|---------------|
| Sin límite duro de FDs | Sigue siendo O(n) al iterar el array |
| No necesita reconstruir set | Copia todo el array al kernel en cada llamada |
| Portabilidad POSIX | Para 100K+ conexiones, overhead significativo |

### `epoll`: la solución Linux de alto rendimiento

```c
#include <sys/epoll.h>

// Crear instancia de epoll
int epfd = epoll_create1(0);

// Registrar socket de escucha
struct epoll_event ev = { .events = EPOLLIN, .data.fd = sfd };
epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &ev);

// Event loop:
struct epoll_event events[64];
while (1) {
    int n = epoll_wait(epfd, events, 64, -1);

    for (int i = 0; i < n; i++) {
        if (events[i].data.fd == sfd) {
            int cfd = accept(sfd, NULL, NULL);
            set_nonblocking(cfd);
            struct epoll_event cev = { .events = EPOLLIN, .data.fd = cfd };
            epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &cev);
        } else {
            handle_client(events[i].data.fd);
        }
    }
}
```

| ✅ Ventajas | ❌ Desventajas |
|------------|---------------|
| O(1) para registrar/desregistrar FDs | Solo Linux |
| Solo devuelve FDs con eventos (no escanea todos) | API más compleja |
| Escala a 100K+ conexiones | Edge-triggered requiere cuidado extra |

### Level-triggered vs Edge-triggered

| Modo | Comportamiento | Riesgo | Simplidad |
|------|---------------|--------|-----------|
| **Level-triggered (LT)** | Mientras haya datos, sigue notificando | Ninguno si procesas parcialmente | ✓ Simple |
| **Edge-triggered (ET)** | Notifica **una vez** al cambiar de estado | Si no drenar todo hasta `EAGAIN`, pierdes el evento | ✗ Más difícil |

> [!TIP]
> **Empieza siempre con level-triggered.** Edge-triggered es una optimización que requiere drenar todo el buffer en cada notificación. Si olvidas leer hasta `EAGAIN`, no recibes más eventos y la conexión "muere" silenciosamente.

---

## Tema 4 — Reactor pattern: la arquitectura

### Estructura de datos por conexión

Cada conexión necesita estado propio:

```c
typedef struct {
    int     fd;
    char    read_buf[4096];     // buffer de lectura (acumulación)
    size_t  read_len;           // bytes acumulados
    char    write_buf[4096];    // buffer de escritura (pendientes)
    size_t  write_len;          // bytes pendientes
    size_t  write_offset;       // bytes ya enviados del write_buf
    int     state;              // estado del protocolo
} connection_t;
```

> [!WARNING]
> **Nunca uses un buffer global compartido entre conexiones.** Si lo haces, un `read` de cliente B sobreescribe datos que cliente A aún no procesó. Cada conexión necesita su propio buffer.

### El event loop

```c
void run_reactor(int listen_fd) {
    connection_t conns[MAX_CONNS];
    struct pollfd pfds[MAX_CONNS + 1];
    int nconns = 0;

    // Registrar listener
    pfds[0] = (struct pollfd){ .fd = listen_fd, .events = POLLIN };

    while (running) {
        // Construir pfds desde conns
        int npfds = 1;  // pfds[0] = listener
        for (int i = 0; i < nconns; i++) {
            short events = POLLIN;
            if (conns[i].write_len > conns[i].write_offset)
                events |= POLLOUT;  // hay datos pendientes por enviar
            pfds[npfds++] = (struct pollfd){
                .fd = conns[i].fd, .events = events
            };
        }

        int ready = poll(pfds, npfds, -1);
        if (ready == -1 && errno == EINTR) continue;

        // Dispatch
        if (pfds[0].revents & POLLIN)
            handle_accept(listen_fd, conns, &nconns);

        for (int i = 0; i < nconns; i++) {
            if (pfds[i+1].revents & POLLIN)
                handle_read(&conns[i]);
            if (pfds[i+1].revents & POLLOUT)
                handle_write(&conns[i]);
            if (pfds[i+1].revents & (POLLERR | POLLHUP))
                handle_disconnect(&conns[i], conns, &nconns, i--);
        }
    }
}
```

---

## Tema 5 — TCP es un stream: framing y parsing

### El mito del "un write = un read"

```c
// Cliente envía:
write(fd, "GET key1\n", 9);
write(fd, "GET key2\n", 9);

// Servidor recibe (posibilidades):
read → "GET key1\nGET key2\n"     // ambos juntos
read → "GET key1\nGE"             // uno completo + fragmento
read → "GET ke"                    // mitad del primero
```

TCP no respeta los límites de tus `write`. Lo que envías como 2 mensajes puede llegar como 1, o como 3 fragmentos, o en cualquier otra combinación.

**Necesitas framing explícito.** Las dos opciones principales:

| Método | Ejemplo | Ventaja | Desventaja |
|--------|---------|---------|------------|
| **Delimitador** | Cada mensaje termina con `\n` | Simple de implementar y debuggear | Necesitas escapar el delimitador si aparece en los datos |
| **Longitud prefijada** | `[4 bytes len][payload]` | Eficiente para datos binarios | Más complejo de parsear incrementalmente |

### Parser incremental por líneas

```c
// Acumular bytes en el buffer de la conexión:
ssize_t n = read(conn->fd, conn->read_buf + conn->read_len,
                 sizeof(conn->read_buf) - conn->read_len);
if (n <= 0) { /* EOF o error */ }
conn->read_len += n;

// Buscar líneas completas:
char *start = conn->read_buf;
char *end;
while ((end = memchr(start, '\n', conn->read_len - (start - conn->read_buf)))) {
    *end = '\0';
    process_command(conn, start);  // procesar línea completa
    start = end + 1;
}

// Mover remanente al inicio del buffer:
size_t remaining = conn->read_len - (start - conn->read_buf);
if (remaining > 0 && start != conn->read_buf) {
    memmove(conn->read_buf, start, remaining);
}
conn->read_len = remaining;
```

> [!CAUTION]
> **Limita el tamaño del buffer.** Si un cliente malicioso envía megabytes sin `\n`, tu buffer crece sin límite (o desborda un buffer estático). Implementa un límite máximo de línea y desconecta al cliente si lo excede:
> ```c
> if (conn->read_len >= sizeof(conn->read_buf)) {
>     fprintf(stderr, "Cliente %d: línea demasiado larga, desconectando\n", conn->fd);
>     close_connection(conn);
> }
> ```

### Protocolo textual key-value (ejemplo del bloque)

```
Solicitud                      Respuesta
─────────                      ─────────
SET key1 valor1\n     →        OK\n
GET key1\n            →        VALUE valor1\n
GET noexiste\n        →        NOT_FOUND\n
DEL key1\n            →        DELETED\n
INVALID\n             →        ERROR unknown command\n
```

---

## Tema 6 — Backpressure: cuando el peer no lee

### El problema

Si el cliente conectado no llama `read`, el buffer TCP del kernel se llena. Tu `write` empieza a escribir parcialmente y eventualmente retorna `EAGAIN`.

```
Tu servidor                Buffer TCP kernel              Cliente
[datos] ──write──▶ [███████████████████] ──────▶ [no lee]
                   ↑ LLENO → EAGAIN                ↑ buffer lleno
```

### Solución: cola de escritura por conexión

```c
int try_write(connection_t *conn) {
    while (conn->write_offset < conn->write_len) {
        ssize_t n = write(conn->fd,
                          conn->write_buf + conn->write_offset,
                          conn->write_len - conn->write_offset);
        if (n == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Buffer TCP lleno: guardar posición, pedir POLLOUT
                return 0;  // datos pendientes
            }
            return -1;  // error real
        }
        conn->write_offset += n;
    }
    // Todo enviado: resetear buffer
    conn->write_len = 0;
    conn->write_offset = 0;
    return 1;  // nada pendiente
}
```

**Claves del manejo de escritura:**
1. Intentar `write` inmediatamente al preparar la respuesta.
2. Si no se envió todo (`EAGAIN`), registrar interés en `POLLOUT`.
3. Cuando el event loop notifica `POLLOUT`, reintentar `try_write`.
4. Cuando todo se envió, **desregistrar** `POLLOUT` (si no, el event loop notifica innecesariamente cada iteración).

---

## Tema 7 — Cierre y ciclo de vida de conexiones

### EOF: `read` retorna 0

```c
ssize_t n = read(conn->fd, buf, sizeof(buf));
if (n == 0) {
    // El peer cerró su lado de escritura (FIN TCP)
    // Puede que aún tengamos datos pendientes por enviar
    flush_pending_writes(conn);
    close(conn->fd);
    remove_from_reactor(conn);
}
```

### Half-close con `shutdown`

TCP permite cerrar solo un lado:

```c
shutdown(fd, SHUT_WR);   // "Terminé de escribir, pero puedo seguir leyendo"
shutdown(fd, SHUT_RD);   // "Terminé de leer"
shutdown(fd, SHUT_RDWR); // equivale a close (pero sin liberar el FD)
```

### Checklist de limpieza al desconectar

1. Cerrar el FD con `close(fd)`.
2. Remover del selector (`epoll_ctl DEL` o marcar como inactivo en el array de `pollfd`).
3. Liberar estado de la conexión (buffers, parser state).
4. **No usar el FD después de cerrarlo.** Si guardas el FD en alguna estructura, ponlo a `-1` inmediatamente.

> [!WARNING]
> **Use-after-close de FDs:** Si cierras FD 7 y luego `accept` te devuelve FD 7 (reutilizado), escribir en "la conexión vieja" en realidad escribe al cliente nuevo. Siempre invalida el FD al cerrar.

---

## Checklist de salida del Bloque 06

- [ ] Crear un servidor TCP que acepte múltiples clientes con `poll` o `epoll`
- [ ] Todos los sockets de clientes están en modo no bloqueante
- [ ] Manejar `EAGAIN`/`EWOULDBLOCK` sin tratar como error
- [ ] Implementar escritura parcial con cola de salida por conexión
- [ ] Parser incremental que no asume "un read = un mensaje"
- [ ] Límite de tamaño de entrada por conexión (defensa contra overflow)
- [ ] Limpieza completa al desconectar (close + remover del reactor + liberar estado)
- [ ] El servidor no se cae si un cliente se desconecta abruptamente

---

## Referencias

| Recurso | Comando |
|---------|---------|
| Sockets | `man 2 socket`, `man 2 bind`, `man 2 listen`, `man 2 accept` |
| I/O no bloqueante | `man 2 fcntl`, `man 7 socket` |
| Select | `man 2 select` |
| Poll | `man 2 poll` |
| Epoll | `man 7 epoll`, `man 2 epoll_create1`, `man 2 epoll_ctl`, `man 2 epoll_wait` |
| Byte order | `man 3 htons`, `man 3 inet_ntop` |
| Shutdown | `man 2 shutdown` |
| Socket options | `man 7 socket` (buscar `SO_REUSEADDR`) |
