# THEORY_CLAUDE.md — Bloque 02: Archivos, I/O y Sistema de Archivos en Linux

> "Everything is a file" no es un slogan, es la decisión de diseño que define Unix. Este bloque te enseña a operar sobre esa abstracción con las syscalls reales del kernel, entender las capas de buffering que `stdio` agrega encima, y construir herramientas que manejan permisos, enlaces e inodos correctamente.

---

## Mapa del Bloque

```
Tema 1: Modelo de archivos  →  VFS, inodos, "todo es archivo"
Tema 2: I/O con syscalls     →  open/read/write/close, partial writes, EINTR
Tema 3: stdio vs syscalls    →  Buffering, cuándo usar cada uno
Tema 4: Permisos y umask     →  mode_t, chmod, chown, ACLs
Tema 5: Metadatos (stat)     →  stat/lstat/fstat, macros de tipo, timestamps
Tema 6: Directorios          →  opendir/readdir, recorrido recursivo seguro
Tema 7: Enlaces e inodos     →  Hard links, symlinks, unlink, semántica real
Tema 8: Proyecto minifind    →  Todo junto en un clon de find
```

---

## Tema 1 — El modelo de archivos en Linux: lo que realmente existe

### La capa VFS

El kernel Linux no trata con ext4 o xfs directamente cuando tu programa llama `open()`. Hay una capa intermedia llamada **VFS** (Virtual File System) que abstrae todos los filesystems en una interfaz unificada:

```
Tu programa
    │
    └─── syscall: open("/tmp/data.txt", O_RDONLY)
              │
              └─── VFS (Virtual File System)
                       │
                       ├─── ext4 (si /tmp es ext4)
                       ├─── xfs  (si /tmp es xfs)
                       └─── tmpfs (si /tmp es tmpfs)
```

### Inodos: la identidad real

Un nombre de archivo (`/tmp/data.txt`) es solo una **entrada de directorio** — un par `(nombre, número de inodo)`. El inodo es la identidad real del objeto:

```
Directorio /tmp/
  ┌──────────────┬──────────┐
  │ Nombre       │ Inodo #  │
  ├──────────────┼──────────┤
  │ data.txt     │ 1048577  │
  │ backup.txt   │ 1048577  │  ← mismo inodo = hard link
  │ link.txt     │ 1048590  │  ← inodo diferente = archivo distinto (o symlink)
  └──────────────┴──────────┘

Inodo #1048577:
  - tipo: archivo regular
  - permisos: 0644
  - propietario: uid 1000
  - tamaño: 4096 bytes
  - bloques de datos: [bloque 55, bloque 56]
  - nlinks: 2  (data.txt y backup.txt apuntan aquí)
```

Consecuencias prácticas:
- `rename()` cambia la entrada del directorio, no el contenido ni el inodo.
- Borrar un nombre (`unlink`) solo decrementa `nlinks`. Los datos desaparecen cuando `nlinks == 0` y nadie lo tiene abierto.

### "Todo es archivo" — con matices

La tabla de file descriptors de un proceso puede contener:

| Tipo | Ejemplo | `read` | `write` | `seek` |
|------|---------|--------|---------|--------|
| Archivo regular | `/tmp/data.txt` | ✓ | ✓ | ✓ |
| Directorio | `/tmp/` | Solo via `readdir` | ✗ | ✗ |
| Pipe / FIFO | `pipe()` | ✓ | ✓ | ✗ |
| Socket | `socket()` | ✓ (`recv`) | ✓ (`send`) | ✗ |
| Dispositivo | `/dev/null` | ✓ | ✓ | depende |
| Terminal | `/dev/tty` | ✓ | ✓ | ✗ |

Comparten API (`open`, `read`, `write`, `close`) pero no se comportan igual. Un `read` de un socket puede bloquear indefinidamente; un `read` de `/dev/urandom` nunca retorna EOF.

---

## Tema 2 — File descriptors y syscalls de I/O

### La tabla de FDs por proceso

Cada proceso arranca con 3 descriptores:

```
┌──────┬──────────────────────────┐
│  FD  │  Apunta a                │
├──────┼──────────────────────────┤
│   0  │  stdin  (teclado o pipe) │
│   1  │  stdout (terminal o pipe)│
│   2  │  stderr (terminal)       │
│   3+ │  abiertos por tu código  │
└──────┴──────────────────────────┘
```

Un FD es un simple `int` — un índice en una tabla del kernel. No es "el archivo", es un **handle** al archivo.

### `open`: la syscall más importante del bloque

```c
#include <fcntl.h>
#include <sys/stat.h>

int fd = open(path, flags, mode);
// Retorna: FD (>=0) o -1 en error
```

**Flags de acceso** (obligatorio elegir uno):

| Flag | Significado |
|------|-------------|
| `O_RDONLY` | Solo lectura |
| `O_WRONLY` | Solo escritura |
| `O_RDWR` | Lectura y escritura |

**Flags de creación** (combinables con `|`):

| Flag | Significado |
|------|-------------|
| `O_CREAT` | Crear si no existe (requiere argumento `mode`) |
| `O_EXCL` | Con `O_CREAT`: fallar si ya existe (creación atómica) |
| `O_TRUNC` | Truncar a 0 bytes si ya existe |
| `O_APPEND` | Escribir siempre al final |

**Flags de comportamiento:**

| Flag | Significado | Cuándo usarlo |
|------|-------------|---------------|
| `O_CLOEXEC` | Cerrar FD al hacer `exec` | **Siempre** (evita fuga de FDs a procesos hijos) |
| `O_NOFOLLOW` | No seguir symlinks | Cuando la seguridad importa |
| `O_NONBLOCK` | No bloquear en `read`/`write` | Sockets, pipes, event loops |

Patrones de uso típicos:

```c
// Lectura de archivo existente
int fd = open(path, O_RDONLY | O_CLOEXEC);

// Crear archivo nuevo (falla si ya existe)
int fd = open(path, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0644);

// Sobreescribir/crear archivo
int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);

// Añadir a archivo de log
int fd = open(logpath, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0644);
```

> [!IMPORTANT]
> **Siempre usa `O_CLOEXEC`.** Sin él, si tu programa hace `fork` + `exec`, el proceso hijo hereda el FD abierto. Esto es una fuga de recursos y potencialmente un agujero de seguridad (el hijo puede leer un archivo que no debería).

### `read` y `write`: la semántica que todos ignoran

```c
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
```

| Retorno | Significado para `read` | Significado para `write` |
|---------|------------------------|--------------------------|
| `> 0` | Bytes leídos (puede ser < count) | Bytes escritos (puede ser < count) |
| `0` | **EOF** — no hay más datos | (no ocurre normalmente) |
| `-1` | Error — consultar `errno` | Error — consultar `errno` |

> [!CAUTION]
> **`write` puede escribir MENOS de lo que pediste.** Esto se llama **partial write**. Si pasas 4096 bytes y `write` retorna 1024, solo se escribieron 1024. Los otros 3072 bytes están en tu buffer esperando. Si ignoras esto, tu archivo queda incompleto silenciosamente.

### El bucle de escritura robusto — código obligatorio

Nunca hagas un solo `write`. Siempre usa un bucle:

```c
// Escribe exactamente n bytes al fd. Retorna 0 si ok, -1 si error.
static int write_all(int fd, const void *buf, size_t n) {
    const unsigned char *p = buf;
    size_t remaining = n;

    while (remaining > 0) {
        ssize_t w = write(fd, p, remaining);
        if (w == -1) {
            if (errno == EINTR) continue;  // interrumpido por señal → reintentar
            return -1;                      // error real
        }
        p += w;
        remaining -= (size_t)w;
    }
    return 0;
}
```

### `EINTR`: por qué tus syscalls pueden fallar sin razón aparente

Si una **señal** llega mientras tu proceso está dentro de una syscall bloqueante (`read`, `write`, `accept`, etc.), el kernel puede interrumpir la syscall y retornar `-1` con `errno == EINTR`. No es un error real — es el kernel diciendo "te despertaron, inténtalo de nuevo".

```c
ssize_t safe_read(int fd, void *buf, size_t count) {
    ssize_t n;
    do {
        n = read(fd, buf, count);
    } while (n == -1 && errno == EINTR);
    return n;
}
```

> [!NOTE]
> En Bloque 3 trabajarás con señales directamente. Cuando eso ocurra, `EINTR` dejará de ser un concepto teórico — lo vivirás en cada ejercicio.

### `close` también puede fallar

```c
if (close(fd) == -1) {
    perror("close");
    // En NFS y algunos filesystems, close puede retornar error tardío
    // indicando que un write anterior en realidad falló
}
```

### Copia robusta de archivos: el `cp` mínimo

```c
int copy_file(const char *src_path, const char *dst_path) {
    int rc = -1;
    int src_fd = -1, dst_fd = -1;
    char buf[65536];  // 64 KB — buen balance entre syscalls y memoria

    src_fd = open(src_path, O_RDONLY | O_CLOEXEC);
    if (src_fd == -1) { perror(src_path); goto cleanup; }

    dst_fd = open(dst_path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
    if (dst_fd == -1) { perror(dst_path); goto cleanup; }

    ssize_t n;
    while ((n = read(src_fd, buf, sizeof(buf))) > 0) {
        if (write_all(dst_fd, buf, (size_t)n) == -1) {
            perror("write");
            goto cleanup;
        }
    }
    if (n == -1) { perror("read"); goto cleanup; }

    rc = 0;

cleanup:
    if (src_fd != -1) close(src_fd);
    if (dst_fd != -1) {
        if (close(dst_fd) == -1 && rc == 0) {
            perror("close dst");
            rc = -1;
        }
    }
    return rc;
}
```

### Tamaño de buffer: qué usar

| Tamaño | Syscalls para 1 MB | Notas |
|--------|-------------------|-------|
| 1 byte | 1,048,576 | Absurdamente lento — una syscall por byte |
| 512 B | 2,048 | Sectores de disco históricos |
| 4 KB | 256 | Tamaño de página de memoria (buena base) |
| 64 KB | 16 | Buen balance para la mayoría de workloads |
| 1 MB | 1 | Pocas syscalls, pero usa más stack/heap |

El sweet spot suele estar entre **4 KB y 64 KB**. Más allá de eso, las ganancias son marginales porque el cuello de botella pasa al disco.

---

## Tema 3 — `stdio` vs syscalls: la capa de buffering

### Qué agrega `stdio`

Cuando usas `fopen`/`fread`/`fwrite`, la librería C envuelve el FD en un `FILE *` con un **buffer en userspace**:

```
Tu programa                     Kernel
┌────────────────────────┐      ┌──────────────┐
│ fputc('A', f)          │      │              │
│ fputc('B', f)          │─────▶│ un solo      │
│ fputc('C', f)          │      │ write(fd,    │
│ ...                    │      │   "ABC...",  │
│ fputc('Z', f) + flush  │      │   4096)      │
└────────────────────────┘      └──────────────┘
     26 llamadas a fputc    →    1 syscall write
```

Sin buffering, cada `fputc` haría una syscall `write(fd, &c, 1)` — 26 cruces de user/kernel boundary.

### Modos de buffering de `stdio`

| Modo | Constante | Comportamiento | Típicamente |
|------|-----------|---------------|-------------|
| Fully buffered | `_IOFBF` | Flush cuando el buffer se llena | Archivos regulares |
| Line buffered | `_IOLBF` | Flush al encontrar `\n` | stdout cuando es terminal |
| Unbuffered | `_IONBF` | Cada escritura es inmediata | stderr |

```c
// Cambiar modo de buffering
char mybuf[8192];
setvbuf(fp, mybuf, _IOFBF, sizeof(mybuf));

// O dejar que stdio asigne su propio buffer
setvbuf(fp, NULL, _IOFBF, 16384);
```

### Cuándo usar cada enfoque

| Situación | Usar syscalls | Usar stdio |
|-----------|---------------|------------|
| Copia byte a byte de archivos | ✗ (millones de syscalls) | ✓ (buffering automático) |
| Parseo de texto línea a línea | Complicado | ✓ (`fgets`) |
| Sockets / pipes / event loops | ✓ (control de FDs, `select`/`poll`) | ✗ (buffering interfiere) |
| I/O con `O_NONBLOCK` | ✓ | ✗ (stdio no lo maneja bien) |
| Escritura binaria de bloques grandes | ✓ (menos overhead) | Da igual |
| Formateo de texto (`printf`-style) | Complicado | ✓ (`fprintf`) |

> [!WARNING]
> **No mezcles syscalls y stdio sobre el mismo FD.** El buffer de `FILE *` y el estado del kernel están desincronizados. Si haces `write(fileno(fp), ...)` y luego `fgets(fp, ...)`, puedes perder datos o leer basura. Si no tienes alternativa, usa `fflush(fp)` antes de cualquier operación directa con el FD.

### Midiendo la diferencia: `strace -c`

```bash
# Cuenta syscalls de un programa
strace -c ./copy_syscalls   input.bin output.bin

# vs
strace -c ./copy_stdio      input.bin output.bin
```

La versión con syscalls y buffer de 1 byte mostrará miles de `read`/`write`. La versión con `fread`/`fwrite` mostrará muchas menos.

---

## Tema 4 — Permisos, umask y seguridad de archivos

### Bits de permisos (`mode_t`)

```
     ┌─ special bits (setuid/setgid/sticky)
     │  ┌─ user rwx
     │  │  ┌─ group rwx
     │  │  │  ┌─ other rwx
     │  │  │  │
    0  7  5  5   ←  octal 0755
       │  │  │
       │  │  └── r-x (read + execute)
       │  └───── r-x (read + execute)
       └──────── rwx (read + write + execute)
```

| Octal | Simbólico | Uso típico |
|-------|-----------|------------|
| `0644` | `rw-r--r--` | Archivos de datos (scripts, config) |
| `0755` | `rwxr-xr-x` | Ejecutables y directorios |
| `0600` | `rw-------` | Archivos privados (claves SSH, secrets) |
| `0700` | `rwx------` | Directorios privados |

### `umask`: la máscara que sustrae permisos

`umask` no añade permisos — los **quita** de lo que `open`/`mkdir` solicitan:

```
Permiso solicitado:  0666  (rw-rw-rw-)
umask del proceso:   0022  (----w--w-)
Resultado:           0644  (rw-r--r--)

Fórmula: resultado = solicitado & ~umask
```

```c
// Ejemplo: crear archivo con permisos exactos
mode_t old_mask = umask(0);          // temporalmente desactivar umask
int fd = open("secret.key", O_WRONLY | O_CREAT | O_EXCL, 0600);
umask(old_mask);                     // restaurar inmediatamente
```

> [!CAUTION]
> Si olvidas restaurar el `umask`, todos los archivos que tu proceso cree después tendran permisos posiblemente demasiado abiertos. Siempre guarda y restaura.

### `chmod` vs `fchmod`: la diferencia de seguridad

```c
// Por path — susceptible a TOCTOU
chmod("/tmp/data.txt", 0600);
// Entre tu stat() y tu chmod(), otro proceso podría haber
// reemplazado data.txt con un symlink a /etc/shadow

// Por FD — seguro contra TOCTOU
int fd = open("/tmp/data.txt", O_RDONLY | O_NOFOLLOW);
fchmod(fd, 0600);  // opera sobre el archivo ya abierto, no sobre la ruta
```

### Bits especiales

| Bit | Octal | En archivos | En directorios |
|-----|-------|-------------|----------------|
| **setuid** | `04000` | Se ejecuta con permisos del dueño | (Raro) |
| **setgid** | `02000` | Se ejecuta con permisos del grupo | Nuevos archivos heredan el grupo del directorio |
| **sticky** | `01000` | (Sin efecto moderno) | Solo el dueño puede borrar sus archivos (`/tmp`) |

> [!WARNING]
> **Setuid con root es peligroso.** Si tu binario tiene setuid root y tiene un bug, un atacante obtiene ejecución como root. Nunca agregues setuid sin una razón absolutamente necesaria.

---

## Tema 5 — Metadatos con `stat`, `lstat`, `fstat`

### `struct stat`: lo que el inodo sabe

```c
#include <sys/stat.h>

struct stat st;
if (lstat(path, &st) == -1) { perror(path); return -1; }
```

| Campo | Tipo | Contenido |
|-------|------|-----------|
| `st_mode` | `mode_t` | Tipo de archivo + bits de permisos |
| `st_ino` | `ino_t` | Número de inodo |
| `st_nlink` | `nlink_t` | Número de hard links |
| `st_uid` | `uid_t` | User ID del propietario |
| `st_gid` | `gid_t` | Group ID |
| `st_size` | `off_t` | Tamaño en bytes (para archivos regulares) |
| `st_blocks` | `blkcnt_t` | Bloques de 512B realmente asignados |
| `st_mtime` | `time_t` | Última modificación del contenido |
| `st_ctime` | `time_t` | Último cambio de metadatos del inodo |
| `st_atime` | `time_t` | Último acceso (a menudo deshabilitado por rendimiento) |

### La diferencia crítica: `stat` vs `lstat` vs `fstat`

```
archivo.txt  ──  inodo #100 (archivo regular, 4096 bytes)
enlace.txt   ──  inodo #200 (symlink, target = "archivo.txt")

stat("enlace.txt",  &st) → st describe inodo #100 (sigue el symlink)
lstat("enlace.txt", &st) → st describe inodo #200 (el symlink mismo)
fstat(fd,           &st) → st describe lo que fd tiene abierto (inmune a TOCTOU)
```

**Regla práctica:**
- Usa `lstat` para herramientas que recorren directorios (`find`, `ls`): necesitas saber si algo es un symlink antes de decidir si seguirlo.
- Usa `fstat` cuando ya tienes el archivo abierto: es más seguro y eficiente.
- Usa `stat` solo cuando explícitamente quieres seguir symlinks.

### Macros de tipo para `st_mode`

```c
if (S_ISREG(st.st_mode))  printf("Archivo regular\n");
if (S_ISDIR(st.st_mode))  printf("Directorio\n");
if (S_ISLNK(st.st_mode))  printf("Symlink\n");       // solo con lstat
if (S_ISFIFO(st.st_mode)) printf("FIFO/named pipe\n");
if (S_ISSOCK(st.st_mode)) printf("Socket\n");
if (S_ISCHR(st.st_mode))  printf("Dispositivo de caracteres\n");
if (S_ISBLK(st.st_mode))  printf("Dispositivo de bloques\n");
```

### Timestamps: `ctime` NO es "creation time"

| Campo | Qué registra | Ejemplo de trigger |
|-------|-------------|-------------------|
| `st_atime` | Último acceso (lectura) | `read()`, `cat file` |
| `st_mtime` | Última modificación de contenido | `write()`, `echo X >> file` |
| `st_ctime` | Último cambio de metadatos | `chmod`, `chown`, `link`, `rename` |

> [!NOTE]
> En muchos Linux modernos, `atime` se actualiza con `relatime` (solo si atime < mtime) para evitar escrituras al disco en cada lectura. Algunos sistemas lo desactivan completamente (`noatime`).

### Extraer permisos legibles de `st_mode`

```c
static void format_permissions(mode_t m, char *buf) {
    // buf debe tener al menos 11 bytes
    buf[0] = S_ISDIR(m) ? 'd' : S_ISLNK(m) ? 'l' : S_ISFIFO(m) ? 'p' : '-';
    buf[1] = (m & S_IRUSR) ? 'r' : '-';
    buf[2] = (m & S_IWUSR) ? 'w' : '-';
    buf[3] = (m & S_IXUSR) ? 'x' : '-';
    buf[4] = (m & S_IRGRP) ? 'r' : '-';
    buf[5] = (m & S_IWGRP) ? 'w' : '-';
    buf[6] = (m & S_IXGRP) ? 'x' : '-';
    buf[7] = (m & S_IROTH) ? 'r' : '-';
    buf[8] = (m & S_IWOTH) ? 'w' : '-';
    buf[9] = (m & S_IXOTH) ? 'x' : '-';
    buf[10] = '\0';
}
// Ejemplo: format_permissions(0100755, buf) → "-rwxr-xr-x"
```

---

## Tema 6 — Directorios: recorrido recursivo seguro

### API base

```c
#include <dirent.h>

DIR *dirp = opendir(path);
if (!dirp) { perror(path); return -1; }

struct dirent *ent;
while ((ent = readdir(dirp)) != NULL) {
    // ent->d_name contiene el nombre del archivo (sin path)
    // ent->d_type puede contener el tipo (DT_REG, DT_DIR, etc.) — NO siempre
}
closedir(dirp);
```

> [!WARNING]
> **`d_type` no siempre está disponible.** En algunos filesystems (XFS antiguos, NFS, algunos FUSE), `d_type == DT_UNKNOWN`. Siempre valida con `lstat` si necesitas el tipo real. No confíes ciegamente en `d_type`.

### Filtrar `.` y `..` — obligatorio en recursión

```c
while ((ent = readdir(dirp)) != NULL) {
    if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        continue;
    // procesar...
}
```

Sin este filtro, tu recursión entra en un bucle infinito: `.` se refiere al directorio actual, `..` al padre.

### Construir rutas hijas: `snprintf`, nunca `strcat`

```c
char child_path[PATH_MAX];
int ret = snprintf(child_path, sizeof(child_path), "%s/%s", parent_path, ent->d_name);
if (ret < 0 || (size_t)ret >= sizeof(child_path)) {
    fprintf(stderr, "Ruta demasiado larga: %s/%s\n", parent_path, ent->d_name);
    continue;  // skip, no crash
}
```

### Recorrido recursivo completo

```c
static int walk_dir(const char *path, int depth, int max_depth) {
    if (max_depth >= 0 && depth > max_depth) return 0;

    struct stat st;
    if (lstat(path, &st) == -1) {
        fprintf(stderr, "minifind: %s: %s\n", path, strerror(errno));
        return 1;  // hubo error, pero no abortamos
    }

    // Evaluar filtros e imprimir si coincide
    if (matches_filters(path, &st)) {
        printf("%s\n", path);
    }

    // Si no es directorio, no recursar
    if (!S_ISDIR(st.st_mode)) return 0;

    DIR *dirp = opendir(path);
    if (!dirp) {
        fprintf(stderr, "minifind: %s: %s\n", path, strerror(errno));
        return 1;
    }

    int err = 0;
    struct dirent *ent;
    while ((ent = readdir(dirp)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;

        char child[PATH_MAX];
        if (snprintf(child, sizeof(child), "%s/%s", path, ent->d_name) >= (int)sizeof(child)) {
            fprintf(stderr, "minifind: ruta demasiado larga, saltando\n");
            err = 1;
            continue;
        }

        err |= walk_dir(child, depth + 1, max_depth);
    }

    closedir(dirp);
    return err;
}
```

### TOCTOU: la vulnerabilidad silenciosa

```c
// INSEGURO:
struct stat st;
stat(path, &st);             // paso 1: verificar
// VENTANA: entre stat y open, alguien reemplaza path con un symlink a /etc/shadow
int fd = open(path, O_RDWR); // paso 2: actuar sobre ruta potencialmente distinta
```

**Time-of-check to time-of-use (TOCTOU)**: entre verificar y actuar, el estado del filesystem puede cambiar.

Mitigaciones:
- Usa `fstat(fd, &st)` después de `open` en vez de `stat` antes.
- Usa `O_NOFOLLOW` para rechazar symlinks en la apertura.
- Usa `openat(dirfd, name, flags)` para operar relativo a un directorio ya abierto.

---

## Tema 7 — Inodos, hard links, symlinks y `unlink`

### Hard links: dos nombres, un archivo

```c
link("original.txt", "alias.txt");
```

```
original.txt ──┐
               ├──▶ inodo #1048577 → datos reales
alias.txt    ──┘

stat("original.txt") → st_ino = 1048577, st_nlink = 2
stat("alias.txt")    → st_ino = 1048577, st_nlink = 2  (idénticos)
```

Restricciones de hard links:
- No pueden cruzar filesystems (el inodo es local al FS).
- No pueden apuntar a directorios (para evitar ciclos en el árbol).

### Symlinks: un archivo que contiene una ruta

```c
symlink("original.txt", "shortcut.txt");
```

```
shortcut.txt ──▶ inodo #2000000 (tipo: symlink)
                 contenido: "original.txt"  ← es literalmente una cadena de texto

original.txt ──▶ inodo #1048577 (tipo: archivo regular)
```

El symlink es un **objeto distinto** que almacena una ruta como texto. Si borras `original.txt`, el symlink queda **roto** (dangling).

### `readlink`: leer el target de un symlink

```c
char target[PATH_MAX];
ssize_t len = readlink(path, target, sizeof(target) - 1);
if (len == -1) { perror("readlink"); return; }
target[len] = '\0';  // readlink NO agrega '\0'
printf("Symlink apunta a: %s\n", target);
```

> [!CAUTION]
> **`readlink` NO null-termina.** Si olvidas `target[len] = '\0'`, `printf("%s", target)` leerá basura hasta encontrar un cero aleatorio en memoria.

### `unlink`: borrar un nombre, no necesariamente el archivo

```c
unlink("alias.txt");
// Si original.txt sigue existiendo → nlinks baja a 1, datos intactos
// Si alias.txt era el último nombre → nlinks = 0 → datos borrados
//    ...PERO si algún proceso tiene el archivo abierto, los datos
//    persisten hasta que cierre el FD
```

### Patrón de archivo temporal seguro

```c
int fd = open("/tmp/myapp.XXXXXX.tmp", O_RDWR | O_CREAT | O_EXCL, 0600);
unlink("/tmp/myapp.XXXXXX.tmp");  // borrar el nombre inmediatamente
// El archivo existe mientras el FD esté abierto, pero nadie más puede abrirlo
write(fd, data, len);
lseek(fd, 0, SEEK_SET);
read(fd, buf, len);
close(fd);  // ahora sí desaparece del disco
```

> [!TIP]
> Para temporales reales, `mkstemp` es más seguro que construir nombres a mano:
> ```c
> char template[] = "/tmp/myapp-XXXXXX";
> int fd = mkstemp(template);  // reemplaza XXXXXX con nombre único
> unlink(template);            // borrar nombre, mantener FD
> ```

### Detectar hard links vs archivos distintos

```c
struct stat st1, st2;
stat("file_a.txt", &st1);
stat("file_b.txt", &st2);

if (st1.st_dev == st2.st_dev && st1.st_ino == st2.st_ino) {
    printf("Son el mismo archivo (hard link)\n");
} else {
    printf("Son archivos distintos\n");
}
```

Necesitas comparar **ambos** `st_dev` (dispositivo) y `st_ino` (inodo), porque dos filesystems distintos podrían tener el mismo número de inodo.

---

## Tema 8 — Proyecto integrador: `minifind`

### Arquitectura recomendada

```
main()
 │
 ├── parse_args(argc, argv)    →  struct con filtros
 │
 └── walk_dir(base_path, filtros, 0)
      │
      ├── lstat(path)           →  obtener metadatos
      ├── matches(path, &st, filtros)  →  evaluar filtros
      │     ├── check_name(name, pattern)   →  fnmatch(3)
      │     ├── check_type(st.st_mode, 'f'/'d'/'l')
      │     └── check_size(st.st_size, spec)
      │
      └── si es directorio: opendir → readdir → recursar
```

### Filtros mínimos

| Flag | Argumento | Qué evalúa | Función de libc |
|------|-----------|-------------|-----------------|
| `-name` | Patrón glob | Nombre base del archivo | `fnmatch(pattern, basename, 0)` |
| `-type` | `f`, `d`, `l` | Tipo de archivo | `S_ISREG`, `S_ISDIR`, `S_ISLNK` |
| `-size` | `Nc` (bytes) | Tamaño en bytes | `st.st_size` |
| `-maxdepth` | Entero | Profundidad máxima | Contador en la recursión |

### Reglas de robustez

1. **No abortar por un error aislado.** Si un directorio tiene permisos denegados, reportar y seguir con el resto.
2. **No seguir symlinks en recursión** (a menos que el usuario lo pida). Evita ciclos infinitos.
3. **Cerrar `DIR *` en cada nivel.** Cada `opendir` consume un FD. Si no cierras, te quedas sin FDs en árboles profundos.
4. **Proteger contra rutas largas.** `PATH_MAX` existe, pero podría no ser suficiente en algunos filesystems. Verifica siempre el retorno de `snprintf`.

### Mensajes de error: el formato Unix

```
minifind: /ruta/problemática: Permission denied
```

Formato: `nombre_programa: ruta_en_cuestión: mensaje_de_strerror(errno)`

---

## Checklist de salida del Bloque 02

- [ ] Implementar copia de archivo robusta con `read`/`write_all` y manejo de `EINTR`
- [ ] Explicar por qué `fread`/`fwrite` con buffer de 1 byte es más rápido que `read`/`write` de 1 byte
- [ ] Predecir el permiso final de `open("f", O_CREAT, 0666)` con `umask(0027)`
- [ ] Distinguir `stat` vs `lstat` y explicar cuándo usar cada uno
- [ ] Detectar si dos rutas son hard links al mismo archivo
- [ ] Construir un recorrido recursivo que no crashee, no haga leaks de FDs, y maneje errores por ruta

---

## Referencias

| Recurso | Comando |
|---------|---------|
| File descriptors | `man 2 open`, `man 2 read`, `man 2 write`, `man 2 close` |
| Stdio | `man 3 fopen`, `man 3 fread`, `man 3 setvbuf` |
| Metadatos | `man 2 stat`, `man 2 lstat`, `man 2 fstat` |
| Directorios | `man 3 opendir`, `man 3 readdir`, `man 3 closedir` |
| Permisos | `man 2 chmod`, `man 2 umask`, `man 2 chown` |
| Enlaces | `man 2 link`, `man 2 symlink`, `man 2 readlink`, `man 2 unlink` |
| Pattern matching | `man 3 fnmatch` |
| Temporales seguros | `man 3 mkstemp` |
