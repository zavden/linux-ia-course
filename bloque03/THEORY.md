# THEORY.md — Bloque 03: Procesos, IPC y Señales en Linux

Este bloque marca el salto de "programa C" a "software de sistema".
A partir de aquí manejas múltiples procesos, reemplazo de imagen (`exec`), comunicación por pipes, señales asíncronas y bases de servicios tipo daemon/shell.

## Alcance del bloque

Debes dominar:

1. Modelo de proceso y `fork`.
2. Recolección de hijos (`wait/waitpid`) y manejo de zombies.
3. Familia `exec*` y herencia de descriptores.
4. Redirección (`dup2`) y pipes anónimos.
5. Señales con `sigaction` y enmascaramiento.
6. Fundamentos de daemonización y control de jobs.
7. Arquitectura base para `minishell`.

Material previo del bloque quedó en `bloque03/ej_legacy/`.

---

## 1) Modelo de proceso real en Linux

## 1.1 Qué es un proceso

Un proceso encapsula:

1. imagen de memoria virtual
2. contexto de CPU (registros, PC, stack)
3. tabla de file descriptors
4. credenciales (UID/GID)
5. estado de señales y scheduling

## 1.2 PID, PPID y árbol de procesos

Cada proceso tiene:

- `pid_t getpid(void)`
- `pid_t getppid(void)`

La jerarquía importa para:

1. recolección de hijos
2. entrega de señales
3. control de jobs en terminal

## 1.3 `fork` y copy-on-write (COW)

`fork()` no "duplica RAM byte a byte inmediatamente".
Modelo moderno:

1. padre e hijo comparten páginas iniciales como read-only.
2. cuando uno escribe, kernel clona solo esa página (COW).

Consecuencia:

- `fork` puede ser barato inicialmente.
- escribir mucha memoria tras `fork` sí cuesta.

Firma:

```c
pid_t fork(void);
```

Retornos:

1. `-1`: error
2. `0`: estás en hijo
3. `>0`: estás en padre (PID del hijo)

## 1.4 Estado global tras `fork`

Se hereda:

1. memoria virtual (COW)
2. FDs abiertos
3. variables de entorno
4. signal dispositions (con matices)

No se comparte "estado vivo" mutable después de fork, salvo recursos explícitamente compartidos (shm, fd sobre pipe/socket, etc.).

## 1.5 Reglas prácticas de seguridad tras `fork`

En procesos multihilo, tras `fork` (en el hijo) se recomienda ejecutar solo funciones async-signal-safe hasta `exec`.
Esto evita deadlocks internos por locks de librerías.

---

## 2) Espera de hijos: `wait` y `waitpid`

## 2.1 Por qué esperar

Si el padre no recoge hijos terminados, quedan zombies (entrada de tabla de procesos sin recursos completos, esperando status).

## 2.2 API base

```c
pid_t wait(int *status);
pid_t waitpid(pid_t pid, int *status, int options);
```

`waitpid` permite control fino:

1. hijo específico
2. cualquier hijo (`-1`)
3. no bloqueante (`WNOHANG`)

## 2.3 Decodificación de `status`

Macros esenciales:

1. `WIFEXITED(status)`
2. `WEXITSTATUS(status)`
3. `WIFSIGNALED(status)`
4. `WTERMSIG(status)`
5. `WIFSTOPPED(status)`
6. `WIFCONTINUED(status)`

Nunca leas `status` "a mano" con máscaras mágicas.

## 2.4 Zombies y huérfanos

1. Zombie: hijo terminado no recolectado.
2. Huérfano: padre termina antes; el hijo es adoptado (normalmente PID 1 / subreaper).

## 2.5 Recolección en shells/daemons

Patrón común:

- handler de `SIGCHLD` que ejecuta loop `while (waitpid(-1, &st, WNOHANG) > 0) { ... }`

---

## 3) Familia `exec*`: reemplazo de imagen

## 3.1 Idea clave

`exec` no crea proceso nuevo: reemplaza la imagen del proceso actual.

Si `exec` tiene éxito:

- no retorna.

Si retorna, falló (revisar `errno`).

## 3.2 Variantes comunes

1. `execl`, `execv`
2. `execlp`, `execvp` (buscan en `PATH`)
3. `execle`, `execve` (control de entorno)

`execve` es la syscall base.

## 3.3 Patrón clásico `fork + exec`

1. padre hace `fork`
2. hijo ajusta redirecciones/FDs
3. hijo hace `execvp`
4. padre espera o sigue (foreground/background)

## 3.4 Herencia de FDs y `FD_CLOEXEC`

Por defecto los FDs abiertos sobreviven a `exec`.
Para evitar fugas de descriptor entre procesos:

- usar `O_CLOEXEC` al abrir
- o `fcntl(fd, F_SETFD, FD_CLOEXEC)`

## 3.5 `exec` y errores típicos

1. `ENOENT`: comando no encontrado
2. `EACCES`: sin permisos de ejecución
3. `ENOEXEC`: formato inválido

Mínimo en hijo:

```c
execvp(...);
perror("execvp");
_exit(127);
```

Usa `_exit` en hijo para evitar flush duplicado de buffers `stdio` heredados.

---

## 4) Redirección y pipes

## 4.1 `dup2` para redirección

`dup2(oldfd, newfd)` hace que `newfd` apunte al mismo objeto abierto que `oldfd`.
Uso típico:

1. `dup2(file_fd, STDOUT_FILENO)` para `>`
2. `dup2(file_fd, STDIN_FILENO)` para `<`

## 4.2 Pipes anónimos

```c
int pipefd[2];
pipe(pipefd); // [0]=read, [1]=write
```

Sirven para flujo unidireccional.
Bidireccional entre dos procesos requiere dos pipes.

## 4.3 Regla crítica: cerrar extremos no usados

Si no cierras extremos sobrantes:

1. no llega EOF
2. procesos pueden bloquear indefinidamente
3. aparecen deadlocks difíciles de diagnosticar

## 4.4 Pipeline de 2 comandos (`A | B`)

Esquema:

1. crear pipe
2. fork hijo A
3. en A: `dup2(pipe_w, STDOUT_FILENO)`
4. fork hijo B
5. en B: `dup2(pipe_r, STDIN_FILENO)`
6. cerrar pipe en padre/hijos según corresponda
7. `exec` en ambos hijos
8. `waitpid` en padre

## 4.5 Errores comunes en IPC

1. olvidar `close(pipe_w)` en lector
2. escribir sin manejar parciales
3. mezclar buffering de `stdio` sin flush adecuado
4. no propagar códigos de error del hijo

---

## 5) Señales: modelo asíncrono

## 5.1 Qué es una señal

Notificación asíncrona de evento al proceso/hilo.
Puede venir de:

1. kernel (ej. `SIGCHLD`, `SIGPIPE`, `SIGSEGV`)
2. usuario/proceso (`kill`, `raise`)

## 5.2 Señales relevantes del bloque

1. `SIGINT` (Ctrl+C)
2. `SIGTERM` (terminación amable)
3. `SIGKILL` (no interceptable)
4. `SIGCHLD` (hijo cambió estado)
5. `SIGHUP` (reload tradicional)
6. `SIGUSR1/2` (uso app)

## 5.3 `sigaction` (en lugar de `signal`)

Estructura:

```c
struct sigaction sa;
sa.sa_handler = handler;
sigemptyset(&sa.sa_mask);
sa.sa_flags = SA_RESTART;
sigaction(SIGINT, &sa, NULL);
```

Ventajas:

1. comportamiento consistente POSIX
2. control de máscara durante handler
3. flags (`SA_RESTART`, `SA_NOCLDSTOP`, etc.)

## 5.4 Async-signal-safety

Dentro del handler no puedes llamar cualquier API.
`printf`, `malloc`, etc. no son seguras en handler.

Seguro mínimo:

1. cambiar flags `volatile sig_atomic_t`
2. usar `write` para mensajes crudos (si necesario)

## 5.5 `sigprocmask` y secciones críticas

Permite bloquear señales temporalmente:

1. construir `sigset_t`
2. `sigprocmask(SIG_BLOCK, ...)`
3. sección crítica
4. `sigprocmask(SIG_UNBLOCK, ...)`

Útil para evitar carreras con handlers al mutar estado compartido.

## 5.6 `SA_RESTART`

Puede reiniciar automáticamente ciertas syscalls interrumpidas.
Aun así, debes diseñar código preparado para `EINTR`.

---

## 6) Foreground, background y grupos de procesos

## 6.1 Job control básico

Terminal entrega señales al process group en foreground.
Por eso `Ctrl+C` normalmente afecta comando en foreground, no todo el sistema.

## 6.2 Shell y procesos en background (`&`)

Al lanzar background:

1. shell no bloquea con wait normal
2. recolección ocurre vía `SIGCHLD` + `waitpid(..., WNOHANG)`

## 6.3 Señales en minishell

Comportamiento esperado:

1. shell padre no debe morir con `Ctrl+C`
2. hijos foreground sí pueden morir con `SIGINT`

Esto exige configurar handlers/dispositions según contexto (padre vs hijo).

---

## 7) Daemonización

## 7.1 Patrón tradicional (doble fork)

Pasos clásicos:

1. `fork` + salir padre
2. `setsid`
3. segundo `fork`
4. `chdir("/")`
5. ajustar `umask`
6. cerrar/redirigir FDs estándar

## 7.2 PID file y exclusión

Para evitar múltiples instancias:

1. lockfile/pidfile
2. validar proceso existente
3. cleanup del pidfile al terminar

## 7.3 Señales de operación

1. `SIGTERM`: shutdown limpio
2. `SIGHUP`: recarga config

## 7.4 Daemon moderno con systemd

En sistemas modernos suele ser mejor delegar en systemd:

1. evita parte del boilerplate de daemonización
2. centraliza logging, restart policy, dependencies

---

## 8) Arquitectura de `minishell`

## 8.1 Componentes mínimos

1. loop REPL
2. parser de línea
3. builtins (`cd`, `exit`, `pwd`, `env/export`)
4. ejecución externa (`fork/exec`)
5. pipeline básico
6. redirecciones
7. background jobs

## 8.2 Builtins vs externos

`cd` debe ejecutarse en el proceso padre shell; si lo haces en hijo, no persiste cambio de directorio.

## 8.3 Pipeline mínimo estable

1. parser separa comando izquierda/derecha
2. crea pipe
3. fork dos hijos
4. dup2 en cada hijo
5. exec en cada rama
6. wait del padre

## 8.4 Manejo de zombies en shell

Handler `SIGCHLD` con recolección no bloqueante evita acumulación de zombies al usar `&`.

## 8.5 Errores y UX mínima

1. comando no encontrado -> mensaje claro
2. retorno de exit status de comandos
3. no crashear ante input vacío

---

## 9) Patrones de robustez de este bloque

## 9.1 Cleanup unificado

Aunque el foco sea procesos, sigues necesitando cleanup consistente de FDs y memoria.

## 9.2 `_exit` en hijos

Tras `fork`, en rutas de error del hijo usa `_exit`, no `exit`, para evitar efectos secundarios de buffers heredados.

## 9.3 Logging con contexto

Mensajes tipo:

```text
minishell: execvp(ls): No such file or directory
```

Nombre de herramienta + operación + causa.

## 9.4 Evitar deadlocks

Checklist en pipes:

1. cierre correcto extremos
2. no esperar proceso que depende de FD que tú no cerraste
3. no mezclar bloqueos circulares padre/hijo

---

## 10) Criterios de dominio del Bloque 03

Debes poder:

1. crear varios hijos con `fork` y recogerlos sin zombies.
2. ejecutar comandos externos con `execvp` y manejo de error correcto.
3. redirigir stdout/stderr usando `dup2`.
4. implementar pipe de una o dos etapas sin deadlock.
5. diseñar handlers con `sigaction` sin anti-patrones graves.
6. construir daemon básico con ciclo de vida limpio.
7. conectar todo en un mini shell funcional.

---

## 11) Referencias técnicas recomendadas

1. `man 2 fork`
2. `man 2 waitpid`
3. `man 3 exec`, `man 2 execve`
4. `man 2 pipe`, `man 2 dup2`
5. `man 2 sigaction`, `man 7 signal`
6. `man 2 sigprocmask`
7. `man 2 setsid`
8. `man 1 ps`, `man 1 kill`

---

## Práctica del bloque

Los ejercicios nuevos del bloque (10 resueltos pedagógicos + 3 complejos) están en:

- `bloque03/EJERCICIOS.md`
- `bloque03/practica/`
