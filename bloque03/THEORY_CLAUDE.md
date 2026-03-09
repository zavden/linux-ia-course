# THEORY_CLAUDE.md — Bloque 03: Procesos, Señales e IPC en Linux

> Este bloque marca la transición de "escribir programas" a "programar sistemas". A partir de aquí, tu código crea procesos hijos, los coordina con pipes, reacciona a eventos asíncronos del kernel (señales), y construye herramientas que se comportan como las utilidades reales del sistema — empezando por un shell interactivo.

---

## Mapa del Bloque

```
Tema 1: Modelo de proceso     →  fork, copy-on-write, herencia de estado
Tema 2: Espera de hijos       →  wait/waitpid, zombies, huérfanos
Tema 3: exec                  →  Reemplazo de imagen, fork+exec, _exit
Tema 4: Redirección y pipes   →  dup2, pipes anónimos, pipelines
Tema 5: Señales               →  sigaction, async-signal-safety, sigprocmask
Tema 6: Job control y daemons →  Foreground/background, daemonización
Tema 7: Proyecto minishell    →  Todo junto
```

---

## Tema 1 — El modelo de proceso en Linux

### Qué es un proceso

Un proceso no es "un programa ejecutándose". Es un **objeto del kernel** que encapsula:

```
Proceso (struct task_struct en el kernel)
├── Espacio de memoria virtual (código, heap, stack, mmap)
├── Tabla de file descriptors (FDs abiertos)
├── Credenciales (UID, GID, grupos suplementarios)
├── Estado de CPU (registros, program counter) — guardado en context switch
├── Señales (pendientes, bloqueadas, handlers)
├── Directorio de trabajo actual (cwd)
├── Variables de entorno
└── PID, PPID, process group, session
```

### PID, PPID y el árbol

```c
pid_t mi_pid   = getpid();    // mi PID
pid_t padre    = getppid();   // PID de quien me creó
```

Todo proceso (excepto PID 1) tiene un padre. Si el padre muere, el hijo es "adoptado" por PID 1 (`init`/`systemd`) o un subreaper designado con `prctl(PR_SET_CHILD_SUBREAPER)`.

### `fork`: crear una copia del proceso

```c
pid_t pid = fork();
```

| Retorno | Estás en... | Significado |
|---------|-------------|-------------|
| `-1` | Padre | Error (`errno` contiene la causa) |
| `0` | **Hijo** | Eres el nuevo proceso |
| `> 0` | **Padre** | El valor es el PID del hijo |

Después de `fork`, hay **dos procesos independientes** ejecutando el mismo código desde la misma línea:

```c
pid_t pid = fork();

if (pid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
} else if (pid == 0) {
    // === HIJO ===
    printf("Soy el hijo, mi PID es %d\n", getpid());
    _exit(0);
} else {
    // === PADRE ===
    printf("Soy el padre, mi hijo es %d\n", pid);
    int status;
    waitpid(pid, &status, 0);
}
```

### Copy-on-Write: fork no duplica toda la memoria

```
Antes del fork:
  Padre: [página A] [página B] [página C]

Después del fork (COW):
  Padre: [página A]─┐  [página B]─┐  [página C]─┐
  Hijo:  [página A]─┘  [página B]─┘  [página C]─┘
         (compartidas, marcadas read-only por el kernel)

Cuando el hijo escribe en página B:
  Padre: [página A]─┐  [página B original]  [página C]─┐
  Hijo:  [página A]─┘  [página B copia]     [página C]─┘
                        (solo esta se duplicó)
```

Consecuencia práctica: `fork()` es barato si el hijo hace `exec` inmediatamente (porque nunca escribe en las páginas heredadas). Pero si el hijo modifica mucha memoria, el kernel copia muchas páginas y el costo crece.

### Qué hereda el hijo

| Se hereda (copia) | NO se hereda |
|--------------------|-------------|
| Memoria virtual (COW) | PID (es nuevo) |
| Tabla de FDs (los mismos FDs apuntan a los mismos objetos abiertos) | Locks de `flock` (no se heredan) |
| Variables de entorno | Timers pendientes |
| Signal dispositions (handlers) | Señales pendientes |
| UID, GID, cwd, umask | Memory locks (`mlock`) |

> [!IMPORTANT]
> **Los FDs se comparten a nivel de kernel.** Padre e hijo tienen entradas duplicadas en sus tablas de FDs que apuntan a la **misma descripción de archivo abierto** (file description). Si el hijo hace `lseek` o `close`, afecta el offset compartido. Esto es fundamental para entender pipes.

---

## Tema 2 — Esperar hijos: `wait`, `waitpid` y zombies

### El ciclo de vida de un proceso

```
fork()           exec()           exit()         wait()
  │                │                │              │
  ▼                ▼                ▼              ▼
CREATED ──────▶ RUNNING ──────▶ ZOMBIE ──────▶ REAPED
                  │    ▲                        (desaparece)
                  │    │
                  ▼    │
              STOPPED ─┘  (señales SIGSTOP/SIGCONT)
```

Un proceso **zombie** es un proceso que ya terminó pero cuya entrada en la tabla de procesos persiste porque el padre no ha llamado `wait`. Ocupa PID y una entrada (pequeña) en el kernel. En un daemon que crea miles de hijos sin recogerlos, los zombies agotan la tabla de procesos.

### `waitpid`: control fino

```c
pid_t waitpid(pid_t pid, int *status, int options);
```

| `pid` | Espera a... |
|-------|-------------|
| `> 0` | El hijo con ese PID específico |
| `-1` | Cualquier hijo |
| `0` | Cualquier hijo del mismo process group |

| `options` | Efecto |
|-----------|--------|
| `0` | Bloquea hasta que un hijo termine |
| `WNOHANG` | Retorna inmediatamente (0 si nadie terminó) |
| `WUNTRACED` | También reporta hijos detenidos (SIGSTOP) |

### Decodificar el status

El `int status` que devuelve `waitpid` **no es el exit code directamente**. Está codificado:

```c
int status;
waitpid(pid, &status, 0);

if (WIFEXITED(status)) {
    // Terminó normalmente con exit()
    int code = WEXITSTATUS(status);  // 0-255
    printf("Salió con código %d\n", code);
}
else if (WIFSIGNALED(status)) {
    // Fue matado por una señal
    int sig = WTERMSIG(status);
    printf("Matado por señal %d (%s)\n", sig, strsignal(sig));
}
else if (WIFSTOPPED(status)) {
    // Detenido (SIGSTOP, SIGTSTP, etc.)
    int sig = WSTOPSIG(status);
    printf("Detenido por señal %d\n", sig);
}
```

> [!CAUTION]
> **Nunca leas `status` directamente con operaciones de bits.** El layout interno es dependiente de la implementación. Siempre usa las macros `WIFEXITED`, `WEXITSTATUS`, etc.

### Recolección de múltiples hijos

Para un shell o daemon que lanza muchos procesos en background:

```c
// En el handler de SIGCHLD o en el loop principal:
void reap_children(void) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status))
            printf("[%d] terminó con código %d\n", pid, WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            printf("[%d] matado por señal %d\n", pid, WTERMSIG(status));
    }
}
```

El `while` con `WNOHANG` es crítico: pueden haber llegado **múltiples** `SIGCHLD` (las señales no se encolan, se fusionan).

---

## Tema 3 — familia `exec`: reemplazar la imagen del proceso

### La idea clave

`exec` no crea un proceso nuevo. **Reemplaza** el código, datos, heap y stack del proceso actual con un nuevo programa. El PID no cambia. Los FDs abiertos (sin `O_CLOEXEC`) se heredan.

```
Antes de exec:         Después de exec:
┌──────────────┐       ┌──────────────┐
│ mi programa  │       │ /bin/ls      │
│ main()       │  ──▶  │ main()       │
│ mis vars     │       │ vars de ls   │
│ mi heap      │       │ heap de ls   │
│ PID: 1234    │       │ PID: 1234    │  ← mismo PID
│ FD 0,1,2,3   │       │ FD 0,1,2     │  ← FD 3 se cerró (O_CLOEXEC)
└──────────────┘       └──────────────┘
```

**Si `exec` tiene éxito, no retorna.** Si retorna, falló.

### Las variantes de `exec`

| Función | Args como... | Busca en PATH | Entorno |
|---------|-------------|---------------|---------|
| `execl` | Lista (`arg0, arg1, ..., NULL`) | No | Hereda |
| `execlp` | Lista | **Sí** | Hereda |
| `execle` | Lista | No | Lo pasas tú |
| `execv` | Array (`char *argv[]`) | No | Hereda |
| `execvp` | Array | **Sí** | Hereda |
| `execve` | Array | No | Lo pasas tú |
| `execvpe` | Array | **Sí** | Lo pasas tú |

**`execvp` es la más usada** (array de args + búsqueda en PATH):

```c
char *args[] = {"ls", "-la", "/tmp", NULL};
execvp(args[0], args);
// Si llegamos aquí, exec falló
perror("execvp");
_exit(127);
```

### El patrón `fork` + `exec`

Esto es **la base de cómo un shell ejecuta comandos**:

```c
pid_t pid = fork();
if (pid == -1) {
    perror("fork");
    return -1;
}

if (pid == 0) {
    // === HIJO: preparar y ejecutar ===

    // Aquí van redirecciones (dup2) si necesarias...

    char *args[] = {"grep", "error", "log.txt", NULL};
    execvp(args[0], args);

    // Si exec retorna, falló
    perror("execvp");
    _exit(127);   // ← _exit, NO exit
}

// === PADRE: esperar ===
int status;
waitpid(pid, &status, 0);
return WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
```

### `_exit` vs `exit`: por qué importa en el hijo

`exit()` ejecuta los handlers de `atexit()` y hace `fflush` de todos los `FILE *`. Pero después de `fork`, el hijo **heredó copias de los buffers de stdio del padre**. Si el hijo llama `exit()`, puede hacer flush de datos que el padre también va a hacer flush → **salida duplicada**.

```c
// INCORRECTO:
if (pid == 0) {
    execvp(...);
    perror("exec");
    exit(1);      // ← puede duplicar output del padre
}

// CORRECTO:
if (pid == 0) {
    execvp(...);
    perror("exec");
    _exit(127);   // ← sale sin flush, sin atexit handlers
}
```

---

## Tema 4 — Redirección y pipes

### `dup2`: redirigir un FD

```c
int dup2(int oldfd, int newfd);
// newfd ahora apunta al mismo archivo/pipe/socket que oldfd
// Si newfd estaba abierto, se cierra primero automáticamente
```

**Redirigir stdout a un archivo** (`comando > output.txt`):

```c
// En el hijo, antes de exec:
int fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
if (fd == -1) { perror("open"); _exit(1); }

dup2(fd, STDOUT_FILENO);  // stdout (FD 1) ahora apunta al archivo
close(fd);                 // ya no necesitamos el FD original

execvp(cmd, args);         // toda la salida de cmd va al archivo
```

**Redirigir stdin desde un archivo** (`comando < input.txt`):

```c
int fd = open("input.txt", O_RDONLY);
dup2(fd, STDIN_FILENO);    // stdin (FD 0) ahora lee del archivo
close(fd);
execvp(cmd, args);
```

### Pipes anónimos: comunicación padre ↔ hijo

```c
int pipefd[2];
pipe(pipefd);
// pipefd[0] = extremo de LECTURA
// pipefd[1] = extremo de ESCRITURA
```

Visualización:

```
                    pipe
pipefd[1] ──────▶ [buffer del kernel] ──────▶ pipefd[0]
(escritura)       (tipicamente 64KB)         (lectura)
```

> [!WARNING]
> **Regla de oro de los pipes: cerrar los extremos que no usas.** Si el proceso lector no cierra `pipefd[1]`, nunca recibirá EOF (porque el kernel ve que alguien todavía podría escribir). Si el proceso escritor no cierra `pipefd[0]`, desperdicias un FD.

### Implementar un pipeline: `ls | grep .c`

```c
int pipefd[2];
if (pipe(pipefd) == -1) { perror("pipe"); return -1; }

// === Hijo izquierdo: ls ===
pid_t pid_left = fork();
if (pid_left == 0) {
    close(pipefd[0]);                   // no leo del pipe
    dup2(pipefd[1], STDOUT_FILENO);     // mi stdout va al pipe
    close(pipefd[1]);                   // ya duplicado, cerrar original
    execlp("ls", "ls", NULL);
    _exit(127);
}

// === Hijo derecho: grep ===
pid_t pid_right = fork();
if (pid_right == 0) {
    close(pipefd[1]);                   // no escribo al pipe
    dup2(pipefd[0], STDIN_FILENO);      // mi stdin viene del pipe
    close(pipefd[0]);                   // ya duplicado, cerrar original
    execlp("grep", "grep", ".c", NULL);
    _exit(127);
}

// === Padre: cerrar pipe y esperar ===
close(pipefd[0]);    // el padre no usa ningún extremo
close(pipefd[1]);

int status;
waitpid(pid_left, &status, 0);
waitpid(pid_right, &status, 0);
```

> [!CAUTION]
> **El padre DEBE cerrar ambos extremos del pipe.** Si el padre no cierra `pipefd[1]`, `grep` nunca recibirá EOF porque el kernel ve que alguien (el padre) todavía podría escribir. El resultado: deadlock silencioso — `grep` espera input para siempre.

### Diagrama de FDs en un pipeline

```
Antes de cerrar extremos sobrantes:

  Padre:     pipefd[0]─┐          ← DEBE cerrar esto
             pipefd[1]─┼─────┐   ← DEBE cerrar esto
                       │     │
  Hijo ls:   pipefd[0] │     │   ← DEBE cerrar esto
             pipefd[1]─┼─┐   │
             stdout dup2 ─┘   │
                       │      │
  Hijo grep: pipefd[0]─┼──┐  │   
             stdin dup2 ───┘  │
             pipefd[1]────────┘   ← DEBE cerrar esto
```

---

## Tema 5 — Señales: eventos asíncronos del kernel

### Qué es una señal

Una señal es una notificación asíncrona enviada a un proceso. Piensa en ella como una **interrupción de software**:

```
Tu código ejecutándose normalmente
        │
        │ ← SIGINT (Ctrl+C): el kernel interrumpe tu código
        ▼
Se ejecuta tu handler de SIGINT
        │
        ▼
Tu código continúa (o termina, según lo que haga el handler)
```

### Señales que debes conocer

| Señal | Número | Origen | Default | Uso |
|-------|--------|--------|---------|-----|
| `SIGINT` | 2 | Ctrl+C en terminal | Terminar | Interrumpir programa interactivo |
| `SIGTERM` | 15 | `kill pid` | Terminar | Pedir terminación amable |
| `SIGKILL` | 9 | `kill -9 pid` | **Terminar (NO interceptable)** | Matar proceso incondicionalmente |
| `SIGCHLD` | 17 | Kernel | Ignorar | Hijo cambió de estado (terminó/paró) |
| `SIGHUP` | 1 | Terminal cerrada | Terminar | Recargar config en daemons |
| `SIGPIPE` | 13 | Escribir a pipe roto | Terminar | Pipe/socket sin lector |
| `SIGSEGV` | 11 | Acceso inválido a memoria | Core dump | Bug en tu código |
| `SIGUSR1/2` | 10/12 | Programador | Terminar | Uso definido por la aplicación |
| `SIGSTOP` | 19 | — | **Detener (NO interceptable)** | Pausar proceso |
| `SIGCONT` | 18 | — | Continuar | Reanudar proceso detenido |

### `sigaction`: el reemplazo correcto de `signal`

```c
#include <signal.h>

// El handler
void handle_sigint(int sig) {
    (void)sig;  // evitar warning de parámetro no usado
    // SOLO operaciones async-signal-safe aquí
}

// Instalación
struct sigaction sa;
sa.sa_handler = handle_sigint;
sigemptyset(&sa.sa_mask);    // no bloquear señales adicionales durante el handler
sa.sa_flags = SA_RESTART;     // reiniciar syscalls interrumpidas automáticamente
if (sigaction(SIGINT, &sa, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
}
```

**¿Por qué no `signal()`?** Porque `signal()` tiene comportamiento **no especificado por POSIX** en aspectos críticos: ¿se resetea el handler después de atraparlo? ¿se reinician syscalls interrumpidas? Con `sigaction`, tú controlas todo explícitamente.

### Async-signal-safety: lo que puedes y no puedes hacer en un handler

Un handler de señal puede interrumpir tu programa en **cualquier punto** — incluso en medio de un `malloc` o `printf`. Si tu handler llama `malloc`, podrías corromper las estructuras internas del allocator.

| ✅ Seguro en handler | ❌ PROHIBIDO en handler |
|----------------------|------------------------|
| Escribir en `volatile sig_atomic_t` | `printf`, `fprintf` |
| `write(fd, ...)` (syscall directa) | `malloc`, `free` |
| `_exit()` | `exit()` |
| `signal()`, `sigaction()` | Cualquier función de stdio |
| Funciones listadas en `man 7 signal-safety` | `syslog`, `strerror` |

**Patrón seguro: el handler solo pone un flag, el loop principal actúa:**

```c
static volatile sig_atomic_t got_sigint = 0;

void handler(int sig) {
    (void)sig;
    got_sigint = 1;
}

int main(void) {
    // instalar handler...

    while (!got_sigint) {
        // trabajo normal
        do_work();
    }

    // cleanup aquí, donde SÍ podemos llamar cualquier función
    printf("Recibí SIGINT, saliendo limpiamente\n");
    cleanup();
    return 0;
}
```

### `sigprocmask`: bloquear señales en secciones críticas

Si tu loop principal modifica una estructura de datos y tu handler de `SIGCHLD` también la lee, hay una carrera. La solución: bloquear la señal temporalmente:

```c
sigset_t block_set, old_set;
sigemptyset(&block_set);
sigaddset(&block_set, SIGCHLD);

// Bloquear SIGCHLD
sigprocmask(SIG_BLOCK, &block_set, &old_set);

// Sección crítica: modificar lista de jobs sin interrupciones
remove_job(pid);

// Restaurar máscara original
sigprocmask(SIG_SETMASK, &old_set, NULL);
// Si SIGCHLD llegó durante el bloqueo, se entrega ahora
```

### `SA_RESTART`: reinicio automático de syscalls

Cuando una señal interrumpe una syscall bloqueante (`read`, `write`, `accept`, etc.), normalmente retorna `-1` con `errno == EINTR`. Con `SA_RESTART`, **algunas** syscalls se reinician automáticamente:

| Con `SA_RESTART` | Sin `SA_RESTART` |
|-------------------|-------------------|
| `read`, `write` se reinician | Retornan `-1` con `EINTR` |
| Pero `select`, `poll`, `epoll_wait` **no se reinician** | No se reinician |

> [!NOTE]
> Incluso con `SA_RESTART`, debes manejar `EINTR` para syscalls que no se reinician. Es más seguro **siempre** tener un loop `while (ret == -1 && errno == EINTR)`.

---

## Tema 6 — Job control y daemonización

### Foreground vs Background

La terminal entrega señales (como `SIGINT` de Ctrl+C) al **process group en foreground**:

```
Terminal (sesión)
├── Shell (session leader, PID 100)
├── Process Group 200 (foreground)
│   └── vim (PID 200)    ← recibe Ctrl+C
└── Process Group 300 (background)
    └── make (PID 300)   ← NO recibe Ctrl+C
```

En un shell:
- `./programa` lo ejecuta en foreground (el shell espera).
- `./programa &` lo ejecuta en background (el shell vuelve al prompt).

### El shell y `SIGINT`

Tu minishell debe:
1. **Ignorar** `SIGINT` en el proceso padre (el shell no debe morir con Ctrl+C).
2. **Restaurar** `SIGINT` a `SIG_DFL` en el hijo **antes de `exec`** (el comando sí debe morir con Ctrl+C).

```c
// En el padre (setup del shell):
signal(SIGINT, SIG_IGN);   // el shell ignora Ctrl+C

// En el hijo, antes de exec:
signal(SIGINT, SIG_DFL);   // restaurar comportamiento normal
execvp(cmd, args);
```

### Recolección de hijos en background

```c
// Handler de SIGCHLD:
void sigchld_handler(int sig) {
    (void)sig;
    int saved_errno = errno;  // salvar errno — el handler puede modificarlo
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // Marcar job como terminado en la tabla de jobs
        // (solo operaciones async-signal-safe!)
    }
    errno = saved_errno;      // restaurar errno
}
```

> [!IMPORTANT]
> **Salvar y restaurar `errno` en handlers de señal.** `waitpid` puede modificar `errno`. Si la señal interrumpe tu código entre un `open` que falló y tu lectura de `errno`, perderás el error original.

### Daemonización: el patrón clásico

Un daemon es un proceso que corre en background sin terminal asociada:

```c
void daemonize(void) {
    // 1. Primer fork: el padre sale, el hijo ya no es líder de grupo
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);   // padre sale

    // 2. Nueva sesión: desconectar de la terminal
    if (setsid() == -1) exit(EXIT_FAILURE);

    // 3. Segundo fork: el hijo no puede volver a adquirir una terminal
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);   // primer hijo sale

    // 4. Directorio de trabajo seguro
    chdir("/");

    // 5. umask limpio
    umask(0);

    // 6. Cerrar FDs estándar y redirigir a /dev/null
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    open("/dev/null", O_RDONLY);   // FD 0 = stdin → /dev/null
    open("/dev/null", O_WRONLY);   // FD 1 = stdout → /dev/null
    open("/dev/null", O_WRONLY);   // FD 2 = stderr → /dev/null
}
```

**Señales de operación en daemons:**
- `SIGTERM` → shutdown limpio (liberar recursos, cerrar conexiones, borrar PID file).
- `SIGHUP` → recargar configuración sin reiniciar.

> [!TIP]
> **En un sistema con systemd**, la daemonización clásica generalmente no es necesaria. Systemd puede ejecutar tu proceso en foreground (`Type=simple`) y manejar el ciclo de vida por ti. El patrón de doble fork sigue siendo útil para entender cómo funciona, pero en producción moderna, déjalo en manos de systemd (Bloque 7).

---

## Tema 7 — Proyecto integrador: `minishell`

### Arquitectura

```
main()
  └── loop REPL
        │
        ├── readline() o fgets()    ← leer input
        │
        ├── parse_line(input)       ← tokenizar
        │     └── detectar: pipes, redirecciones, &
        │
        ├── is_builtin(cmd)?
        │     ├── cd:     chdir(arg)    ← DEBE ejecutarse en el padre
        │     ├── exit:   break loop
        │     ├── pwd:    getcwd() + printf
        │     ├── export: setenv()
        │     └── env:    environ
        │
        └── execute_external(cmd, args)
              │
              ├── sin pipe:
              │     fork → [redirect] → exec → wait
              │
              └── con pipe (A | B):
                    pipe() → fork A → fork B → close fds → wait both
```

### Builtins: por qué `cd` no puede ser un proceso hijo

```c
// INCORRECTO:
if (pid == 0) {
    chdir(path);   // cambia el cwd del HIJO
    _exit(0);
}
// El padre sigue en el directorio anterior

// CORRECTO:
if (strcmp(cmd, "cd") == 0) {
    if (chdir(path) == -1) perror("cd");
    return;   // ejecutar en el padre
}
```

`chdir` modifica el cwd **del proceso que lo llama**. Si lo haces en un hijo, el padre no se entera.

### Manejo de señales en el shell

| Señal | En el padre (shell) | En el hijo (comando) |
|-------|---------------------|----------------------|
| `SIGINT` | Ignorar (no morir) | Default (sí morir) |
| `SIGQUIT` | Ignorar | Default |
| `SIGCHLD` | Handler para reap background jobs | Default |
| `SIGTSTP` | Ignorar (o manejar para job control avanzado) | Default (permite Ctrl+Z) |

### Errores comunes en shells

| Error | Síntoma | Causa |
|-------|---------|-------|
| Zombies acumulándose | `ps aux` muestra procesos `Z` | No recoger hijos con `wait` |
| Pipeline se congela | Shell no regresa al prompt | Padre no cerró extremo de escritura del pipe |
| `cd` no funciona | El directorio no cambia | `cd` ejecutado en hijo en vez de padre |
| Input vacío crashea | Segfault al parsear | No verificar que `fgets` retornó NULL o string vacío |
| Ctrl+C mata el shell | El shell termina | `SIGINT` no ignorado en el padre |

---

## Checklist de salida del Bloque 03

- [ ] Crear N hijos con `fork` y recogerlos a todos sin dejar zombies
- [ ] Ejecutar un comando externo con `execvp` y manejar el error si no existe
- [ ] Redirigir stdout/stderr de un hijo a archivos usando `dup2`
- [ ] Implementar un pipeline de dos comandos (`A | B`) sin deadlock
- [ ] Instalar un handler con `sigaction` que sea async-signal-safe
- [ ] Bloquear/desbloquear señales con `sigprocmask` en secciones críticas
- [ ] Explicar por qué se usa `_exit` en vez de `exit` en hijos post-fork
- [ ] Construir un minishell con builtins, ejecución externa, pipes y manejo de Ctrl+C

---

## Referencias

| Recurso | Comando |
|---------|---------|
| Procesos | `man 2 fork`, `man 2 _exit` |
| Espera | `man 2 waitpid`, `man 2 wait` |
| Ejecución | `man 3 exec`, `man 2 execve` |
| Redirección | `man 2 dup2`, `man 2 pipe` |
| Señales | `man 2 sigaction`, `man 7 signal`, `man 7 signal-safety` |
| Máscara de señales | `man 2 sigprocmask`, `man 3 sigsetops` |
| Daemons | `man 2 setsid`, `man 7 daemon` |
| Shell utils | `man 3 getcwd`, `man 3 chdir` |
