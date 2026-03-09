# THEORY.md — Bloque 02: Archivos, I/O y Sistema de Archivos en Linux

Este bloque convierte la idea "todo es un archivo" en ingeniería práctica.
Aquí ya no basta con que el programa "funcione": debe manejar errores parciales, metadatos reales, permisos, enlaces y recorridos de directorio sin romperse.

## Alcance del bloque

Debes salir de este bloque con dominio de:

1. I/O de bajo nivel con file descriptors (`open/read/write/close`).
2. Diferencias reales entre syscalls y `stdio` (buffering y rendimiento).
3. Permisos, `umask`, `chmod`, ownership y nociones de ACL.
4. Metadatos (`stat/lstat/fstat`) y recorrido de directorios (`opendir/readdir`).
5. Inodos, hard links, symlinks y semántica de `unlink`.
6. Base técnica para implementar `minifind` con criterios sólidos.

Material anterior del bloque quedó en `bloque02/ej_legacy/`.

---

## 1) Modelo de archivos en Linux: lo que realmente existe

## 1.1 VFS e inodos

El kernel ofrece una capa unificada (VFS, Virtual File System) para múltiples FS (ext4, xfs, btrfs, etc.).
A nivel conceptual:

1. El nombre (`/tmp/a.txt`) es una entrada de directorio.
2. El inodo es la identidad real del objeto.
3. Los datos y metadatos viven asociados al inodo.

Por eso:

- Cambiar nombre (`rename`) no cambia contenido.
- Múltiples nombres pueden apuntar al mismo inodo (hard links).

## 1.2 "Todo es archivo" con matices

En Unix, interfaces diversas comparten abstracción de FD:

1. archivo regular
2. directorio
3. pipe/FIFO
4. socket
5. dispositivo (`/dev/*`)

No significa que todos se comporten idéntico, pero sí que muchas operaciones usan la misma API base (`open`, `read`, `write`, `close`, `poll`, etc.).

## 1.3 Espacio de usuario vs espacio kernel

Toda syscall cruza frontera user/kernel.
Cruzar esa frontera tiene costo; hacerlo millones de veces en micro-operaciones puede destruir rendimiento.

Conclusión operativa:

- operaciones grandes y menos frecuentes -> mejor throughput.
- operaciones minúsculas y frecuentes -> overhead alto.

---

## 2) File descriptors y syscalls de I/O

## 2.1 FD: tabla por proceso

Cada proceso tiene una tabla de file descriptors:

- `0`: stdin
- `1`: stdout
- `2`: stderr
- `>=3`: abiertos por el programa

El FD es un índice entero, no "el archivo" en sí.

## 2.2 `open` y flags críticos

Firma:

```c
int open(const char *path, int flags, ... /* mode_t mode */);
```

Flags importantes:

1. acceso: `O_RDONLY`, `O_WRONLY`, `O_RDWR`
2. creación: `O_CREAT`, `O_EXCL`, `O_TRUNC`
3. comportamiento: `O_APPEND`, `O_NONBLOCK`, `O_CLOEXEC`, `O_NOFOLLOW` (seguridad)

Ejemplo robusto de salida nueva:

```c
int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
```

## 2.3 `read`/`write`: semántica real

```c
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
```

Retornos:

- `>0`: bytes transferidos
- `0` en `read`: EOF
- `-1`: error (`errno`)

Punto clave: `write` puede escribir menos de `count` bytes (partial write).
Nunca asumas que un solo `write` vacía todo el buffer.

## 2.4 Bucle robusto de escritura

Patrón obligatorio:

```c
size_t off = 0;
while (off < n) {
    ssize_t w = write(fd, buf + off, n - off);
    if (w == -1) {
        if (errno == EINTR) continue;
        // error real
    }
    off += (size_t)w;
}
```

## 2.5 `EINTR` y reintentos

Syscalls bloqueantes pueden ser interrumpidas por señales.
Si falla con `errno == EINTR`, normalmente reintentas.

## 2.6 `close` también puede fallar

Siempre revisa `close(fd)`.
En flujos críticos, error en close puede implicar writeback fallido tardío.

## 2.7 Copy file robusto (`cp` simplificado)

Checklist mínimo:

1. validar argumentos.
2. abrir origen lectura.
3. abrir destino creación/truncado con modo correcto.
4. bucle `read` + bucle de `write` parcial.
5. cleanup ordenado aunque algo falle a mitad.

## 2.8 Umbral de buffer

Valores típicos prácticos para copia secuencial:

- 4 KiB
- 16 KiB
- 64 KiB

No hay valor universal; depende del FS, caché y entorno.

---

## 3) `stdio` vs syscalls: buffering y costo real

## 3.1 Qué agrega `stdio`

`FILE *` introduce buffering en espacio de usuario.
Reduce syscalls cuando consumes datos pequeños (ej. carácter por carácter).

Ejemplo:

- `fgetc` en loop no necesariamente hace una syscall por char.
- usa buffer interno y recarga por bloques.

## 3.2 Tipos de buffer

`stdio` maneja modos típicos:

1. fully buffered
2. line buffered
3. unbuffered

Se pueden ajustar con `setvbuf`.

## 3.3 Cuándo usar cada enfoque

Usa syscalls directas cuando:

1. necesitas control preciso de FDs y flags.
2. trabajas con sockets/pipes/event loops.
3. quieres minimizar capas en caminos críticos de bytes.

Usa `stdio` cuando:

1. procesas texto y parsing lineal.
2. la ergonomía pesa más que control granular.
3. quieres APIs de formato (`fprintf`, `fgets`, etc.).

## 3.4 Mezclar `stdio` y syscalls sobre el mismo FD

Puede producir inconsistencias por buffers desincronizados.
Si debes mezclar, hazlo con extremo cuidado (flush/sync explícitos).

## 3.5 Benchmark sin autoengaño

Reglas:

1. usa `clock_gettime(CLOCK_MONOTONIC)`.
2. calienta caché con warm-up.
3. ejecuta varias corridas y reporta promedio/mediana.
4. misma carga, mismo archivo, mismo entorno.
5. no comparar debug (`-O0`) contra release (`-O2`) y sacar conclusiones.

## 3.6 Métricas útiles

1. tiempo total (ms)
2. throughput (MB/s)
3. cantidad de syscalls (si usas `strace -c` en Linux)

---

## 4) Permisos, umask y seguridad de archivos

## 4.1 Modo de archivo (`mode_t`)

Bits clásicos Unix:

- user/group/other: `rwx`.
- octal típico: `0644`, `0755`, etc.

Ejemplos:

1. `0644`: rw-r--r--
2. `0755`: rwxr-xr-x

## 4.2 `umask`: máscara de creación

`umask` no "suma" permisos, los quita.

Permiso final de creación (simplificado):

- para archivos: `requested & ~umask`

Si pides `0666` y `umask` es `0022`, el resultado será `0644`.

## 4.3 Uso correcto de `umask` en programas

`umask` afecta proceso completo.
Si la modificas temporalmente:

1. guarda valor previo
2. restaura al final

Patrón:

```c
mode_t old = umask(0);
/* crear recursos */
umask(old);
```

## 4.4 `chmod`, `fchmod`, `chown`

1. `chmod(path, mode)` cambia por ruta.
2. `fchmod(fd, mode)` reduce riesgos TOCTOU porque opera sobre FD abierto.
3. `chown`/`fchown` suele requerir privilegios para cambiar ownership arbitrario.

## 4.5 Bits especiales

1. setuid
2. setgid
3. sticky bit

No los uses sin comprender impacto de seguridad.

## 4.6 ACL (visión inicial)

ACL extiende modelo rwx simple con reglas por usuario/grupo específicas.
No reemplaza permisos clásicos; los complementa.

## 4.7 Errores frecuentes de permisos

1. confiar en permisos solicitados ignorando `umask`.
2. crear archivos sensibles con modo permisivo (`0666`) sin justificar.
3. usar `system("chmod ...")` cuando podrías usar syscall directa.

---

## 5) Metadatos: `stat`, `lstat`, `fstat`

## 5.1 Qué contienen

`struct stat` incluye, entre otros:

1. `st_mode` (tipo + permisos)
2. `st_size` (bytes)
3. `st_uid`, `st_gid`
4. `st_nlink`
5. `st_mtime`
6. `st_ino`

## 5.2 Diferencia clave `stat` vs `lstat`

1. `stat(path, ...)` sigue symlink y devuelve datos del target.
2. `lstat(path, ...)` devuelve datos del propio symlink.

Para herramientas tipo `find/ls` frecuentemente necesitas `lstat` para clasificar sin desreferenciar.

## 5.3 Macros de tipo en `st_mode`

1. `S_ISREG`
2. `S_ISDIR`
3. `S_ISLNK`
4. `S_ISCHR`, `S_ISBLK`, etc.

No compare `st_mode` a números mágicos; usa macros.

## 5.4 Time fields

Timestamps comunes:

1. `atime` acceso
2. `mtime` modificación contenido
3. `ctime` cambio de metadatos/inodo

`ctime` NO significa "creation time" en Unix clásico.

---

## 6) Directorios y recorrido seguro

## 6.1 API base

```c
DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);
```

`readdir` retorna puntero reutilizable internamente; si necesitas persistir nombre, cópialo.

## 6.2 Ignorar `.` y `..`

En recorridos recursivos, debes saltarlos para evitar bucles infinitos.

## 6.3 Construcción de rutas

Usa `snprintf` con buffer suficiente.
Evita `strcat` ingenuo sin control de capacidad.

## 6.4 Recursión controlada

Al bajar directorios:

1. valida tipo con `lstat`.
2. evita seguir symlinks si no quieres ciclos.
3. considera profundidad máxima opcional (`maxdepth`).

## 6.5 Riesgo TOCTOU

Patrón inseguro clásico:

1. `stat(path)`
2. luego `open(path)`

Entre ambos pasos otro proceso podría reemplazar `path`.
Mitigación avanzada:

- `openat`, `fstatat`, `O_NOFOLLOW`, operar por FD cuando sea posible.

## 6.6 Escalabilidad

Recorrer árboles grandes requiere:

1. evitar alocaciones excesivas por entrada.
2. controlar profundidad.
3. manejo claro de errores por ruta (log + continuar).

---

## 7) Inodos, hard links, symlinks y `unlink`

## 7.1 Hard link (`link`)

Crea una nueva entrada de directorio al mismo inodo.
Consecuencias:

1. mismo `st_ino`
2. `st_nlink` aumenta
3. borrar un nombre no borra datos mientras `st_nlink > 0`

## 7.2 Symlink (`symlink`)

Es un objeto distinto que almacena una ruta textual al target.
Consecuencias:

1. inodo distinto al target
2. puede quedar roto si target desaparece

## 7.3 `readlink`

Lee el contenido textual del symlink.
Importante: no agrega `\0`; debes manejar buffer manualmente.

## 7.4 `unlink`

Elimina una entrada de nombre.
Para archivo regular:

- el contenido se borra físicamente cuando no quedan enlaces y nadie lo tiene abierto.

## 7.5 Open file + unlink

Caso clásico Unix:

1. proceso abre archivo
2. otro hace `unlink`
3. proceso aún puede leer/escribir por FD abierto

Útil para temporales seguros.

## 7.6 Errores típicos con enlaces

1. confundir identidad por nombre en vez de por inodo.
2. asumir que symlink y hardlink son equivalentes.
3. usar `stat` cuando necesitabas `lstat`.

---

## 8) Manejo de errores en filesystem

## 8.1 `errno` más comunes en este bloque

1. `ENOENT`: ruta inexistente
2. `EACCES`/`EPERM`: permisos
3. `EEXIST`: ya existe (con `O_EXCL`)
4. `ENOTDIR`: componente no es directorio
5. `EISDIR`: operación incompatible con directorio
6. `ENOSPC`: sin espacio
7. `EMFILE`: demasiados FDs abiertos

## 8.2 Estrategia para herramientas CLI

1. reportar error contextual por ruta.
2. continuar cuando tenga sentido (ej. `find` ante archivo inaccesible).
3. retornar código final no-cero si hubo al menos un error.

## 8.3 Mensajes útiles

Formato recomendado:

```text
minifind: /ruta/problemática: Permission denied
```

Nombre de herramienta + ruta + razón de sistema.

---

## 9) Diseño de `minifind` (integrador del bloque)

## 9.1 Requisitos funcionales base

1. base path (default `.`)
2. filtro `-name`
3. filtro `-type` (`f`, `d`, `l`)
4. filtro `-size` exacto en bytes (`Nc`)

## 9.2 Arquitectura sugerida

Separar en componentes:

1. parseo CLI
2. estructura de filtros
3. función recursiva `walk(path, filters)`
4. función `matches(path, st, filters)`
5. reporting de errores

## 9.3 Recorrido recursivo estable

Flujo:

1. `lstat(path, &st)`
2. evaluar filtros y posiblemente imprimir
3. si directorio, `opendir` + `readdir`
4. para cada hijo (excepto `.`/`..`), construir subpath y recursar

## 9.4 Reglas de robustez

1. no abortar todo por un permiso denegado aislado.
2. no seguir symlinks salvo requisito explícito.
3. liberar recursos en cada nivel (closedir, buffers).
4. proteger contra path demasiado largo.

## 9.5 Extensiones naturales futuras

1. `-maxdepth`
2. `-mtime`
3. glob real (`fnmatch`)
4. acciones (`-print`, `-delete`, `-exec`)

---

## 10) Seguridad y calidad en código de filesystem

## 10.1 Recomendaciones prácticas

1. usa `O_CLOEXEC` por defecto en aperturas nuevas.
2. para operaciones sensibles, prefiere variantes por FD (`fstat`, `fchmod`).
3. evita concatenación insegura de rutas.
4. valida rangos y formatos de argumentos de usuario.

## 10.2 Rutas y entrada no confiable

Rutas pueden contener:

1. espacios
2. bytes no imprimibles
3. componentes maliciosos (`../`)

No asumas forma "bonita" de nombres.

## 10.3 Atomicidad y operaciones seguras

1. `rename` dentro del mismo FS suele ser atómico.
2. crear temporal + `fsync` + rename es patrón común para escritura segura.

## 10.4 Criterio de producción

Un programa de este bloque está "bien" cuando:

1. maneja partial I/O.
2. no fuga FDs/memoria.
3. reporta errores útilmente.
4. mantiene semántica coherente ante casos borde.

---

## 11) Criterios de dominio del Bloque 02

Debes ser capaz de:

1. implementar copia de archivos robusta con syscalls.
2. explicar por qué y cuándo `stdio` mejora rendimiento.
3. predecir permiso final de un archivo con `umask` dado.
4. distinguir `stat` vs `lstat` correctamente.
5. demostrar experimentalmente diferencia entre hardlink y symlink.
6. construir un recorrido recursivo tipo `find` sin bucles ni crashes.

Si alguno de estos puntos falla, en bloques de procesos/redes/seguridad los errores serán más costosos.

---

## 12) Referencias técnicas recomendadas

1. `man 2 open`, `man 2 read`, `man 2 write`, `man 2 close`
2. `man 3 fopen`, `man 3 fread`, `man 3 setvbuf`
3. `man 2 stat`, `man 2 lstat`, `man 2 fstat`
4. `man 3 opendir`, `man 3 readdir`, `man 3 closedir`
5. `man 2 chmod`, `man 2 umask`, `man 2 chown`
6. `man 2 link`, `man 2 symlink`, `man 2 readlink`, `man 2 unlink`
7. `man 3 errno`, `man 3 perror`, `man 3 strerror`
8. `man 1 find`, `man 1 ls`, `man 1 stat`

---

## Práctica del bloque

Los ejercicios nuevos del bloque (10 resueltos pedagógicos + 3 complejos) están en:

- `bloque02/EJERCICIOS.md`
- `bloque02/practica/`
