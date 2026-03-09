# 📖 THEORY.md — Bloque 06: Redes y Multiplexación C (Asynchronous I/O)

¿Alguna vez te has preguntado cómo NGINX o NodeJS (escrito en C/C++ por debajo) manejan **diez mil (10,000)** conexiones de usuarios simultáneos en 1 solo núcleo CPU sin crashear tu servidor? `epoll()` es el mayor secreto guardado del Alto Rendimiento en Arquitectura Linux.

---

## 1. El Falso Dios: Sockets Bloqueantes (Blocking Sockets)
Por defecto, todo socket POSIX en C (`int client = accept(...)` o `read(client)`) es Bloqueante. Si invocas `read()` sobre una conexión lenta de un celular rural 3G con lag, **El Servidor Entero (El Hilo de C) se parará en seco** a esperar el byte durante 5,000 milisegundos.
- **Solución Pobre**: ¡Un Hilo por Cliente! (El Block 05 miniserver de la currícula usó un Thread-Pool de 4 para compensar C).
- **Problema Real**: 1 Hilo = 8 MB RAM. 10,000 Clientes simultaneos x 8 MB = 80 GB RAM de puro costo base solo para pausarse en un "Read". La PC explotará OOM-Killed.

---

## 2. La API Diosa (Non-Blocking I/O y Epoll)
En la programación asíncrona no abrimos 10,000 Hilos. Dejamos que 1 solo HIlo (Event-Loop `while(1)`) administre las 10,000 conexiones sin trabarse!
- **`fcntl(fd, F_SETFL, O_NONBLOCK)`**: Transforma al socket en modo Flash. Si tú le haces `read()` y el del pueblo rural 3G no ha enviado todo aún, el Kernel OS detendrá tú `read` y en nanosegundos arrojará un error `EAGAIN` falso, dejándote seguir a la línea de abajo y atender al siguiente cliente en el loop!.
- **`epoll`**: Imagina iterar manualmnete un `while` sobre un array de 10k fds para ver `quién carajo ha subido texto`. ¡Quema mucho CPU!
  La API `epoll` resuelve esto. Creas un `epoll_fd` global, y le susurras al Kernel de OS (con `epoll_ctl`): "Agrégame estos 10k sockets a la lista. Mantenlos dormidos".
  El kernel Linux que está atado a tu Hardware de Antena Wi-Fi sabe cuándo viaja la corriente por los cables.  
  Luego tú en C ejecutas  `epoll_wait()`. Esta función colapsa a tu Papa al 0% CPU pero mágicamente en milésimas de segundo retorna dándote **UN ARRAY PERFECTO C sólo con los FD/Usuarios que SÍ tienen bytes listos para usar su respectivo `read()`. ¡Eso es NGINX y NodeJS C/C++ Core!

---

## 3. Multiplexación Activa Clásica: `select()` y `poll()`
`epoll` nació en los linux modernos en 2.6. Antes de ello, el mundo UNIX completo dependía de la primitiva `select()` (Aún es cross-plataforma Mac/Windows universal).
- **`select()`**: Le pasas una Bitmap Array larguísima C con los pines (fds) y un timeout. Tarda O(N) lo que la hace lenta si el array sube de 1024 clientes.
- **`poll()`**: La evolución directa usando arrays reales structs C. Acepta arrays infinitamente largos, pero sigue siendo lenta a gran escala C OS Kernell porque el Kernell tiene que mirar todo el array en "User Space" uno a uno.
