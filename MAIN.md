# 🐧 Curso Linux-C: Programación en C para Sistemas Linux

> **Nivel objetivo:** LPIC-2 + RHCSA equivalente
> **Distros:** Fedora & Debian (Docker)
> **Metodología:** 5 ejercicios → 1 proyecto real (ciclo repetido)

---

## 📋 Tabla de Contenidos

1. [Bloque 0 — Entorno y Docker](#bloque-0--entorno-y-docker)
2. [Bloque 1 — Fundamentos de C en Linux](#bloque-1--fundamentos-de-c-en-linux)
3. [Bloque 2 — Archivos y Sistema de Archivos](#bloque-2--archivos-y-sistema-de-archivos)
4. [Bloque 3 — Procesos y Señales](#bloque-3--procesos-y-señales)
5. [Bloque 4 — Memoria](#bloque-4--memoria)
6. [Bloque 5 — Hilos y Concurrencia](#bloque-5--hilos-y-concurrencia)
7. [Bloque 6 — Redes y Sockets](#bloque-6--redes-y-sockets)
8. [Bloque 7 — Administración de Sistemas con C](#bloque-7--administración-de-sistemas-con-c)
9. [Bloque 8 — Almacenamiento Avanzado](#bloque-8--almacenamiento-avanzado)
10. [Bloque 9 — Seguridad](#bloque-9--seguridad)
11. [Bloque 10 — Servicios de Red](#bloque-10--servicios-de-red)
12. [Bloque 11 — Kernel y Bajo Nivel](#bloque-11--kernel-y-bajo-nivel)
13. [Bloque 12 — Proyecto Final Integrador](#bloque-12--proyecto-final-integrador)

---

## 🏗️ Estructura de cada Ejercicio

```
bloqueXX/
├── ejNN/
│   ├── README.md          # Enunciado, teoría mínima, criterios de éxito
│   ├── Dockerfile.fedora   # Imagen Fedora para el ejercicio
│   ├── Dockerfile.debian   # Imagen Debian para el ejercicio
│   ├── docker-compose.yml  # Orquestación (si aplica)
│   ├── src/                # Código fuente
│   ├── tests/              # Tests automatizados (scripts bash o C)
│   └── solucion/           # Solución de referencia (oculta inicialmente)
└── proyectoNN/
    ├── README.md
    ├── Dockerfile.fedora
    ├── Dockerfile.debian
    ├── docker-compose.yml
    ├── src/
    └── tests/
```

**Convención de Docker**: Cada ejercicio puede probarse así:
```bash
# Fedora
docker build -f Dockerfile.fedora -t ej01-fedora . && docker run --rm ej01-fedora

# Debian
docker build -f Dockerfile.debian -t ej01-debian . && docker run --rm ej01-debian
```

---

## 🐳 Bloque 0 — Entorno y Docker

> **Objetivo:** Montar el entorno de desarrollo. Aprender Docker como herramienta de laboratorio.

### Ej 0.1 — Hello Docker
- Crear un `Dockerfile.fedora` y `Dockerfile.debian` que compileen y ejecuten un `hello.c`.
- **Fedora:** `FROM fedora:latest` → `dnf install -y gcc make`
- **Debian:** `FROM debian:latest` → `apt-get update && apt-get install -y gcc make`
- Verificar que `./hello` imprime `"Hello from <distro>"` detectando `/etc/os-release`.

### Ej 0.2 — Makefile Básico
- Escribir un `Makefile` con targets: `all`, `clean`, `run`, `test`.
- El programa debe compilar con `-Wall -Wextra -Werror -pedantic -std=c17`.
- Flags de debug: `-g -O0 -fsanitize=address`.
- Verificar que funciona en ambas imágenes.

### Ej 0.3 — Docker Compose Multi-Distro
- Crear `docker-compose.yml` que levante dos servicios: `fedora` y `debian`.
- Ambos compilan y ejecutan el mismo código fuente montado como volumen.
- Verificar output idéntico en ambas distros.

### Ej 0.4 — Valgrind y GDB en Docker
- Instalar `valgrind` y `gdb` en las imágenes.
- Escribir un programa con un memory leak intencional.
- Usar `valgrind --leak-check=full` para detectarlo.
- Documentar cómo hacer `docker run -it --cap-add=SYS_PTRACE` para GDB.

### Ej 0.5 — Variables de Entorno y Configuración
- Programa que lee config desde variables de entorno (`getenv()`).
- Usar `docker run -e VAR=valor` para inyectar configuración.
- Leer también desde archivo de config como fallback.

### 🚀 Proyecto 0 — "build-lab": Sistema de Build Multi-Distro
- Script/Makefile que automatiza: build → test → report en Fedora y Debian.
- Genera un reporte comparativo (JSON o texto) con: compilador usado, versión, warnings, resultado de tests.
- Usa `docker-compose` para orquestar todo.
- **Utilidad real:** Framework reutilizable para todos los ejercicios del curso.

---

## 📦 Bloque 1 — Fundamentos de C en Linux

> **Objetivo:** Dominar C moderno (C17) con énfasis en convenciones POSIX/Linux.

### Ej 1.1 — Argumentos y getopt
- Programa que parsea opciones: `-v` (verbose), `-o <file>` (output), `-n <num>`.
- Usar `getopt()` y `getopt_long()`.
- Imprimir uso (`--help`) con formato estándar de man pages.

### Ej 1.2 — Strings y Buffers Seguros
- Implementar funciones: `safe_strcpy()`, `safe_strcat()`, `safe_snprintf()`.
- Cada una previene buffer overflow.
- Suite de tests que verifica truncamiento correcto y null-termination.

### Ej 1.3 — Structs y Listas Enlazadas
- Implementar lista enlazada genérica (`void *data`).
- Operaciones: `push`, `pop`, `find`, `delete`, `foreach`, `destroy`.
- Tests con diferentes tipos de datos.

### Ej 1.4 — Manejo de Errores POSIX
- Wrapper functions que verifican retornos de syscalls.
- Macro `CHECK(call)` que imprime error con `perror()` y `__FILE__`, `__LINE__`.
- Patrones: goto cleanup, errno checking, strerror_r.

### Ej 1.5 — Aritmética de Punteros y Arrays
- Implementar `my_memcpy()`, `my_memmove()`, `my_memset()`.
- Comparar rendimiento con las de `<string.h>` usando `clock_gettime()`.
- Generar tabla de tiempos para diferentes tamaños.

### 🚀 Proyecto 1 — "miniecho" + "minicat": Clon de `echo` y `cat`
- **miniecho:** Soporta `-n` (no newline), `-e` (escape sequences: `\n`, `\t`, `\\`, `\xHH`).
- **minicat:** Soporta `-n` (numerar líneas), `-b` (solo no-vacías), `-s` (squeeze blank), `-` (stdin).
- Ambos deben pasar tests comparando output con los originales.
- Docker: testear con archivos de diferentes encodings montados como volúmenes.

---

## 📁 Bloque 2 — Archivos y Sistema de Archivos

> **Objetivo:** I/O de bajo nivel (syscalls) y alto nivel (stdio). Permisos, enlaces, atributos.
> **Cert:** LPIC-2 (filesystem), RHCSA (permisos, ACLs, SELinux contexts en archivos).

### Ej 2.1 — open/read/write/close
- Programa que copia archivos usando solo syscalls (sin stdio).
- Experimentar con diferentes tamaños de buffer (1B, 512B, 4KB, 64KB).
- Medir rendimiento con `clock_gettime(CLOCK_MONOTONIC)`.

### Ej 2.2 — stdio vs syscalls
- Mismo programa de copia con `fopen/fread/fwrite/fclose`.
- Benchmark comparativo. Generar gráfica ASCII de tiempos.
- Explicar cuándo usar cada uno.

### Ej 2.3 — Permisos, chmod, chown y umask
- Programa que crea archivos con permisos específicos.
- Implementar equivalente a `chmod` simbólico (e.g., `u+rwx,g-w`).
- Experimentar con umask dentro del contenedor.
- **RHCSA:** Introducir ACLs con `setfacl`/`getfacl` (llamadas desde C con `system()` o libacl).

### Ej 2.4 — Directorios: opendir, readdir, stat
- Programa que lista un directorio recursivamente (como `ls -lR`).
- Mostrar: permisos (formato simbólico), tamaño, fecha, nombre.
- Usar `lstat()` para manejar symlinks correctamente.

### Ej 2.5 — Enlaces duros y simbólicos
- Programa que crea, detecta y resuelve enlaces.
- `link()`, `symlink()`, `readlink()`, `realpath()`.
- Mostrar inode numbers para demostrar enlaces duros.

### 🚀 Proyecto 2 — "minifind": Clon de `find`
- Buscar archivos por: nombre (glob), tamaño, tipo, permisos, fecha modificación.
- Opciones: `-name`, `-size`, `-type`, `-perm`, `-mtime`, `-maxdepth`.
- Acciones: `-print` (default), `-exec cmd {} \;`, `-delete`.
- Docker: crear filesystem complejo en el contenedor para testing.
- Comparar output con `find` real en Fedora y Debian.

---

## ⚙️ Bloque 3 — Procesos y Señales

> **Objetivo:** fork, exec, wait, señales, daemons. Base de administración de sistemas.
> **Cert:** LPIC-2 (system startup, process management), RHCSA (services, jobs).

### Ej 3.1 — fork y wait
- Programa que crea N hijos, cada uno imprime su PID y sale con código diferente.
- Padre recolecta exit status con `waitpid()` y `WEXITSTATUS()`.
- Experimentar con procesos zombie (hijo muere, padre no hace wait).

### Ej 3.2 — exec family
- Programa que ejecuta comandos externos: `execvp()`, `execle()`.
- Implementar un pipeline de 2 comandos (ej: `ls | grep .c`).
- Redirección de stdout/stderr con `dup2()`.

### Ej 3.3 — Pipes y comunicación entre procesos
- Pipe anónimo: padre envía datos, hijo los procesa.
- Named pipe (FIFO): dos programas independientes se comunican.
- Pipe bidireccional usando dos pipes.

### Ej 3.4 — Señales
- Manejadores para: `SIGINT`, `SIGTERM`, `SIGCHLD`, `SIGUSR1`, `SIGUSR2`.
- Usar `sigaction()` (no `signal()`). Explicar por qué.
- Programa que ignora SIGINT 3 veces y al 4to se cierra con cleanup.
- `sigprocmask()` para bloquear señales durante secciones críticas.

### Ej 3.5 — Daemon
- Crear un daemon que: fork doble, `setsid()`, cierra fds, escribe PID file.
- El daemon escribe un log cada N segundos.
- Responde a `SIGHUP` recargando configuración y a `SIGTERM` con graceful shutdown.

### 🚀 Proyecto 3 — "minishell": Shell Interactivo Básico
- Prompt con directorio actual. Parsear comandos con argumentos.
- Built-ins: `cd`, `exit`, `pwd`, `export`, `env`.
- Ejecución externa con `fork/exec`.
- Pipes (`|`), redirecciones (`>`, `>>`, `<`), background (`&`).
- Manejo de `SIGINT` (no mata el shell), `SIGCHLD` (reap zombies).
- Docker: testear con scripts de comandos automatizados.

---

## 🧠 Bloque 4 — Memoria

> **Objetivo:** Gestión de memoria: heap, mmap, shared memory.
> **Cert:** LPIC-2 (capacity planning, resource limits).

### Ej 4.1 — malloc/calloc/realloc/free
- Implementar un array dinámico (`dynarray`) que crece automáticamente.
- Factor de crecimiento 1.5x vs 2x: benchmark comparativo.
- Detectar leaks con Valgrind en Docker.

### Ej 4.2 — mmap: Archivos mapeados en memoria
- Mapear un archivo grande con `mmap()`, buscar un patrón.
- Comparar velocidad vs `read()` en loop.
- `MAP_PRIVATE` vs `MAP_SHARED`: demostrar la diferencia.

### Ej 4.3 — Memoria compartida POSIX
- `shm_open()`, `ftruncate()`, `mmap()` con `MAP_SHARED`.
- Dos procesos: uno escribe, otro lee (sincronizados con semáforos POSIX).
- Docker: dos contenedores compartiendo memoria vía `--ipc=host`.

### Ej 4.4 — Resource Limits
- Programa que consulta y modifica limits: `getrlimit()`, `setrlimit()`.
- Experimentar con `RLIMIT_AS`, `RLIMIT_NOFILE`, `RLIMIT_NPROC`.
- Docker: usar `--ulimit` y comparar con `prlimit` del host.
- **LPIC-2:** `/etc/security/limits.conf`, `ulimit` en bash.

### Ej 4.5 — Pool Allocator
- Implementar un memory pool: pre-alloca un bloque grande, subdivide.
- API: `pool_create(size, chunk_size)`, `pool_alloc()`, `pool_free()`, `pool_destroy()`.
- Benchmark vs `malloc()` para muchas allocaciones pequeñas.

### 🚀 Proyecto 4 — "minitop": Monitor de Procesos
- Leer `/proc/[pid]/stat`, `/proc/[pid]/status`, `/proc/meminfo`, `/proc/loadavg`.
- Mostrar: PID, user, CPU%, MEM%, command, estado.
- Refresh cada N segundos con `ncurses` o ANSI escape codes.
- Ordenar por CPU o memoria. Filtrar por usuario.
- Docker: ejecutar dentro del contenedor con `--pid=host` para ver procesos del host.

---

## 🔄 Bloque 5 — Hilos y Concurrencia

> **Objetivo:** pthreads, mutexes, condition variables, thread pools.

### Ej 5.1 — pthreads Básico
- Crear N hilos, cada uno procesa una porción de un array.
- `pthread_create()`, `pthread_join()`, pasar args con struct.
- Medir speedup vs versión secuencial.

### Ej 5.2 — Mutex y Race Conditions
- Contador compartido sin mutex → demostrar race condition.
- Agregar `pthread_mutex_t` → resultado correcto.
- Benchmark: lock granular vs lock global.

### Ej 5.3 — Condition Variables
- Problema productor-consumidor con buffer circular.
- `pthread_cond_wait()`, `pthread_cond_signal()`, `pthread_cond_broadcast()`.
- Múltiples productores y consumidores.

### Ej 5.4 — Read-Write Locks
- `pthread_rwlock_t`: múltiples lectores simultáneos, un escritor exclusivo.
- Simular caché con lecturas frecuentes y escrituras raras.
- Comparar rendimiento vs mutex simple.

### Ej 5.5 — Semáforos POSIX
- Semáforos nombrados (`sem_open`) y no nombrados (`sem_init`).
- Implementar el problema de los filósofos comensales.
- Docker: dos contenedores sincronizados con semáforos nombrados.

### 🚀 Proyecto 5 — "miniserver": Servidor HTTP Multithreaded
- Thread pool con N workers configurable.
- Servir archivos estáticos de un directorio.
- GET requests: parsear HTTP/1.1, Content-Type por extensión, Content-Length.
- Responses: 200, 404, 403, 500. Log a archivo.
- Docker: exponer puerto, testear con `curl` desde ambas distros.

---

## 🌐 Bloque 6 — Redes y Sockets

> **Objetivo:** Programación de sockets TCP/UDP. Protocolos. Multiplexing.
> **Cert:** LPIC-2 (network config), RHCSA (firewalld, network).

### Ej 6.1 — Socket TCP Básico
- Cliente-servidor echo: cliente envía línea, servidor la devuelve.
- `socket()`, `bind()`, `listen()`, `accept()`, `connect()`.
- IPv4 e IPv6 (`AF_INET`, `AF_INET6`).

### Ej 6.2 — Socket UDP
- Cliente-servidor de broadcast: servidor envía mensajes, múltiples clientes reciben.
- `sendto()`, `recvfrom()`, `setsockopt(SO_BROADCAST)`.
- Docker: red bridge entre contenedores.

### Ej 6.3 — select/poll/epoll
- Servidor que maneja múltiples clientes sin threads.
- Implementar con `select()`, luego `poll()`, luego `epoll()` (Linux-specific).
- Benchmark: 100, 1000, 10000 conexiones simuladas.

### Ej 6.4 — DNS Resolver
- Usar `getaddrinfo()` para resolver nombres.
- Implementar resolución manual: parsear `/etc/resolv.conf`, enviar query DNS (UDP port 53).
- Docker: configurar DNS custom en el contenedor.

### Ej 6.5 — Firewall desde C
- Usar `iptables`/`nftables` vía `system()` o librería.
- **RHCSA (Fedora):** Interactuar con `firewalld` via D-Bus o CLI.
- **Debian:** Equivalente con `nftables` o `ufw`.
- Programa que abre/cierra puertos dinámicamente.

### 🚀 Proyecto 6 — "minicurl": Cliente HTTP
- Resolución DNS, conexión TCP, envío de request HTTP/1.1.
- Soportar: GET, HEAD. Headers: Host, User-Agent, Connection.
- Parsear response: status code, headers, body.
- Descargar archivos con barra de progreso.
- Seguir redirects (301, 302) hasta N niveles.
- Docker: levantar un nginx en un contenedor, hacer requests desde otro.

---

## 🛠️ Bloque 7 — Administración de Sistemas con C

> **Objetivo:** Herramientas de sysadmin escritas en C. Logs, usuarios, servicios.
> **Cert:** RHCSA (users, services, logs), LPIC-2 (system maintenance).

### Ej 7.1 — Syslog y Journald
- Escribir logs con `syslog()` / `openlog()`.
- Leer logs: parsear `/var/log/syslog` (Debian) o usar `sd_journal_*` API (Fedora).
- Docker: configurar logging driver, montar socket de journal.

### Ej 7.2 — Gestión de Usuarios
- Leer `/etc/passwd`, `/etc/shadow` (con permisos), `/etc/group`.
- Usar `getpwnam()`, `getpwuid()`, `getgrnam()`.
- Crear programa que lista usuarios con más detalle que `id`.

### Ej 7.3 — Systemd desde C
- Usar D-Bus API (`sd-bus`) para: listar units, start/stop services, check status.
- **Fedora:** directo con `systemd-devel`.
- **Debian:** equivalente con `libsystemd-dev`.
- Programa que monitorea el estado de un servicio.

### Ej 7.4 — Cron y Timers
- Parsear formato crontab. Determinar próxima ejecución.
- Crear un daemon que ejecuta tareas programadas (mini-cron).
- **RHCSA/LPIC-2:** Comparar cron vs systemd timers.

### Ej 7.5 — Gestión de Paquetes (info)
- **Fedora:** Leer base de datos RPM con `librpm` — listar paquetes instalados, verificar integridad.
- **Debian:** Leer base de datos DPKG — parsear `/var/lib/dpkg/status`.
- Programa que muestra info de un paquete en ambas distros.

### 🚀 Proyecto 7 — "miniservice": Daemon con Systemd Unit
- Daemon completo: PID file, logging a syslog, config file, graceful reload/shutdown.
- Crear `miniservice.service` (unit file) para systemd.
- El servicio monitorea un directorio y ejecuta acciones cuando aparecen archivos nuevos (como `incron`).
- Docker: contenedores con systemd (`--privileged` o `--cgroupns=host`).
- Tests: instalar, start, stop, restart, reload, status en ambas distros.

---

## 💾 Bloque 8 — Almacenamiento Avanzado

> **Objetivo:** LVM, RAID, filesystems, quotas.
> **Cert:** LPIC-2 (advanced storage), RHCSA (LVM, mount, fstab).

### Ej 8.1 — Información de Filesystems
- `statvfs()` para obtener info de un filesystem montado.
- Leer `/proc/mounts` y `/etc/fstab`.
- Programa tipo `df` simplificado.

### Ej 8.2 — Extended Attributes y ACLs
- `setxattr()`, `getxattr()`, `listxattr()`, `removexattr()`.
- ACLs con `libacl`: `acl_get_file()`, `acl_set_file()`.
- Docker: `--cap-add SYS_ADMIN` para xattrs en el contenedor.

### Ej 8.3 — inotify: Monitoreo de Filesystem
- Monitorear un directorio: creación, modificación, eliminación de archivos.
- `inotify_init()`, `inotify_add_watch()`, `read()`.
- Recursivo: monitorear subdirectorios dinámicamente.

### Ej 8.4 — LVM desde C
- Usar `liblvm2` o llamar a `lvm` commands vía `popen()`.
- Programa que lista PVs, VGs, LVs con detalles.
- Docker: crear loop devices y simular LVM (`--privileged`).
- **RHCSA:** Crear, extender, reducir logical volumes.

### Ej 8.5 — Disk Quotas
- Habilitar quotas en un filesystem dentro de Docker.
- `quotactl()` para establecer y consultar quotas.
- Programa que reporta uso vs quota de cada usuario.
- **Fedora:** XFS quotas (project quotas).
- **Debian:** ext4 quotas (user/group quotas).

### 🚀 Proyecto 8 — "minilvm": Gestor de Almacenamiento Visual
- TUI (ncurses) que muestra: discos, particiones, VGs, LVs, mount points, uso.
- Permite crear/extender/eliminar LVs interactivamente.
- Muestra quotas por usuario.
- Docker: entorno privilegiado con loop devices simulando discos.

---

## 🔒 Bloque 9 — Seguridad

> **Objetivo:** Seguridad de sistemas: SELinux/AppArmor, crypto, capabilities, secure coding.
> **Cert:** RHCSA (SELinux), LPIC-2 (system security).

### Ej 9.1 — Linux Capabilities
- `capget()`, `capset()`, `prctl(PR_SET_KEEPCAPS)`.
- Programa que droppea todas las capabilities excepto `CAP_NET_BIND_SERVICE`.
- Docker: `--cap-drop ALL --cap-add NET_BIND_SERVICE`.

### Ej 9.2 — SELinux (Fedora) / AppArmor (Debian)
- **Fedora:** `libselinux` — `getcon()`, `setcon()`, `security_check_context()`.
- **Debian:** AppArmor — parsear perfiles, verificar enforcement.
- Programa que muestra el contexto de seguridad de un proceso/archivo.

### Ej 9.3 — Crypto con OpenSSL
- Hashing: SHA-256 de un archivo.
- Cifrado simétrico: AES-256-GCM encrypt/decrypt de un archivo.
- API de OpenSSL: `EVP_DigestInit/Update/Final`, `EVP_EncryptInit/Update/Final`.

### Ej 9.4 — Secure Coding Practices
- Programa que demuestra y corrige: buffer overflow, format string, integer overflow, TOCTOU.
- Compilar con `-fstack-protector-strong`, `-D_FORTIFY_SOURCE=2`, `-pie`, `-fPIE`.
- Verificar con Valgrind, ASan, UBSan.

### Ej 9.5 — PAM (Pluggable Authentication Modules)
- Usar `libpam` para autenticar un usuario programáticamente.
- `pam_start()`, `pam_authenticate()`, `pam_end()`.
- Docker: configurar PAM modules custom en el contenedor.

### 🚀 Proyecto 9 — "minivault": Gestor de Secretos
- Almacenar key-value pairs cifrados con AES-256-GCM.
- Master password derivada con PBKDF2 (OpenSSL).
- CLI: `set <key>`, `get <key>`, `list`, `delete <key>`, `export`, `import`.
- Archivo de datos con formato binario propio (header + encrypted entries).
- Docker: testear con diferentes usuarios y permisos.

---

## 🌍 Bloque 10 — Servicios de Red

> **Objetivo:** DNS, Web, Mail, NFS/Samba — interacción y administración desde C.
> **Cert:** LPIC-2 (DNS, web, email, file sharing), RHCSA (NFS, Samba).

### Ej 10.1 — DNS: Zona Parser
- Parsear archivos de zona DNS (formato BIND).
- Registros: A, AAAA, CNAME, MX, NS, SOA, TXT.
- Docker: levantar `bind9`/`named` en contenedor, cargar zona custom.

### Ej 10.2 — HTTPS con TLS
- Extender el miniserver del Bloque 5 con TLS usando OpenSSL.
- Generar certificados self-signed en el Dockerfile.
- `SSL_CTX_new()`, `SSL_new()`, `SSL_accept()`, `SSL_read()`, `SSL_write()`.

### Ej 10.3 — SMTP Client
- Conectar a servidor SMTP, enviar email (protocolo raw).
- HELO → MAIL FROM → RCPT TO → DATA → QUIT.
- Docker: levantar `postfix` en contenedor, verificar email recibido.

### Ej 10.4 — NFS Client
- Montar un export NFS programáticamente (`mount()` syscall con `nfs` type).
- Listar exports de un servidor NFS.
- Docker: servidor NFS en un contenedor, cliente en otro.
- **RHCSA:** Configuración equivalente de `/etc/fstab` y `autofs`.

### Ej 10.5 — Samba/CIFS desde C
- Usar `libsmbclient` para listar shares, leer/escribir archivos.
- Docker: servidor Samba en contenedor, cliente en otro.
- **RHCSA:** Montar CIFS shares con credenciales.

### 🚀 Proyecto 10 — "miniproxy": Reverse Proxy HTTP
- Escuchar en un puerto, forwarding requests a backends configurados.
- Config file con: `backend <nombre> <host>:<port>`, `route <path> <backend>`.
- Health checks periódicos a cada backend.
- Logging de requests con timestamp, método, path, status, latencia.
- Docker Compose: proxy + 2 backends (miniserver del Bloque 5).

---

## 🧬 Bloque 11 — Kernel y Bajo Nivel

> **Objetivo:** Namespaces, cgroups, eBPF. Entender Docker por dentro.
> **Cert:** LPIC-2 (kernel, capacity planning).

### Ej 11.1 — Namespaces
- Crear proceso en nuevo namespace: `clone()` con `CLONE_NEWPID`, `CLONE_NEWNET`, `CLONE_NEWNS`.
- Verificar aislamiento: el proceso ve PID 1, red separada, mount separado.
- Docker: `--privileged` para permitir manipulación de namespaces.

### Ej 11.2 — Cgroups v2
- Crear cgroup, asignar proceso, establecer límites de CPU y memoria.
- Escribir en `/sys/fs/cgroup/...`: `memory.max`, `cpu.max`.
- Programa que consume memoria gradualmente hasta ser killed por el cgroup.

### Ej 11.3 — seccomp
- Filtrar syscalls permitidas con `seccomp-bpf`.
- Programa que solo puede hacer: `read`, `write`, `exit`, `sigreturn`.
- Docker: `--security-opt seccomp=profile.json`.

### Ej 11.4 — Kernel Parameters
- Leer/escribir `/proc/sys/...` y `/sys/...`.
- Programa que tunea: `vm.swappiness`, `net.core.somaxconn`, `fs.file-max`.
- Docker: `--sysctl` para configurar parámetros del kernel.
- **LPIC-2:** `sysctl` y `/etc/sysctl.conf`.

### Ej 11.5 — Módulos del Kernel (informativo)
- Listar módulos cargados leyendo `/proc/modules`.
- Obtener info de un módulo con `finit_module()` info (sin cargar).
- Programa tipo `lsmod` + `modinfo` simplificado.
- Docker: requiere `--privileged` y kernel headers del host.

### 🚀 Proyecto 11 — "minicontainer": Runtime de Contenedores
- Crear un contenedor con namespaces (PID, NET, MNT, UTS) + chroot.
- Limitar recursos con cgroups v2 (CPU, memoria).
- Filesystem overlay simplificado.
- CLI: `minicontainer run --image <dir> --mem 128M --cpu 50 -- <cmd>`.
- Docker: ironía máxima — un contenedor dentro de un contenedor (`--privileged`).

---

## 🏆 Bloque 12 — Proyecto Final Integrador

> **Objetivo:** Combinar todo lo aprendido en un proyecto profesional.

### 🚀 Proyecto Final — "minicloud": Plataforma de Microservicios

**Componentes:**

1. **minicloud-registry** — Registro de servicios (DNS-based service discovery).
2. **minicloud-gateway** — API Gateway / Reverse Proxy con load balancing.
3. **minicloud-monitor** — Monitoreo de recursos (CPU, mem, disco, red) de cada servicio.
4. **minicloud-vault** — Gestión de secretos para los servicios.
5. **minicloud-runner** — Ejecuta servicios en contenedores aislados (namespaces + cgroups).

**Requisitos técnicos:**
- Todo en C17. Compilar en Fedora y Debian.
- Comunicación inter-servicio por sockets TCP.
- Configuración por archivos + variables de entorno.
- Logging centralizado vía syslog.
- Tests automatizados para cada componente.
- Docker Compose para orquestar todo el stack.
- Documentación con man pages (`groff`).

**Entregables:**
- Código fuente completo con Makefiles.
- Docker Compose que levanta todo el stack.
- Suite de tests que verifica la integración.
- Documentación: README, man pages, architecture diagram.

---

## 📊 Mapeo a Certificaciones

### RHCSA (EX200) — Temas cubiertos

| Tema RHCSA | Bloque(s) | Ejercicio(s) / Proyecto(s) |
|---|---|---|
| Gestión de archivos y permisos | B2 | Ej 2.3, 2.4, 2.5, Proy 2 |
| SELinux | B9 | Ej 9.2 |
| Firewall (firewalld) | B6 | Ej 6.5 |
| LVM | B8 | Ej 8.4, Proy 8 |
| NFS/Samba | B10 | Ej 10.4, 10.5 |
| Systemd services | B7 | Ej 7.3, 7.4, Proy 7 |
| Usuarios y grupos | B7 | Ej 7.2 |
| Networking | B6 | Ej 6.1–6.5 |
| Gestión de paquetes | B7 | Ej 7.5 |
| Logs y journald | B7 | Ej 7.1 |

> **Nota RHCSA:** Temas exclusivos de Red Hat (como `subscription-manager`, examen en RHEL) se documentan con equivalentes Debian donde sea posible.

### LPIC-2 (201 + 202) — Temas cubiertos

| Tema LPIC-2 | Bloque(s) | Ejercicio(s) / Proyecto(s) |
|---|---|---|
| Capacity planning | B4 | Ej 4.4, Proy 4 |
| Linux kernel | B11 | Ej 11.1–11.5, Proy 11 |
| System startup (systemd) | B7 | Ej 7.3, 7.4, Proy 7 |
| Filesystem y dispositivos | B2, B8 | Ej 2.1–2.5, 8.1–8.5 |
| Advanced storage (LVM, RAID) | B8 | Ej 8.4, Proy 8 |
| Network configuration | B6 | Ej 6.1–6.5 |
| System maintenance | B7 | Ej 7.1–7.5 |
| DNS | B10 | Ej 10.1 |
| Web servers | B5, B10 | Proy 5, Ej 10.2 |
| File sharing (NFS/Samba) | B10 | Ej 10.4, 10.5 |
| Email (SMTP) | B10 | Ej 10.3 |
| System security | B9 | Ej 9.1–9.5, Proy 9 |

---

## 📐 Resumen Cuantitativo

| Métrica | Cantidad |
|---|---|
| Bloques | 13 (0–12) |
| Ejercicios | 60 |
| Proyectos reales | 13 |
| Dockerfiles | ~120 (2 por ejercicio/proyecto) |
| Certificaciones cubiertas | LPIC-2, RHCSA |
| Distros soportadas | Fedora, Debian |

---

## 🗺️ Prerrequisitos

- **Linux básico:** Terminal, navegación de archivos, editor de texto (vim/nano).
- **C básico:** Variables, funciones, if/for/while, punteros (se refuerza en Bloque 1).
- **Docker instalado:** Docker Engine + Docker Compose en el host (Fedora o Debian).
- **LPIC-1 nivel** (recomendado, no obligatorio).

---

## 🚦 Cómo usar este curso

1. **Secuencial:** Los bloques están ordenados por dependencia. No saltarse.
2. **Docker siempre:** Cada ejercicio se ejecuta en contenedores. No ensuciar el host.
3. **Dual-distro:** Siempre compilar y testear en Fedora Y Debian.
4. **Proyectos obligatorios:** Son la evaluación real. Los ejercicios son preparación.
5. **Git:** Versionar todo. Un commit por ejercicio completado.

```
git init
git add bloque01/ej01/
git commit -m "B01-E01: Hello Docker completado"
```
