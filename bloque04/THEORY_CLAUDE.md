# THEORY_CLAUDE.md — Bloque 04: Memoria, Mapeos y Límites en Linux

> En los bloques anteriores usaste memoria. En este bloque aprendes a **diseñarla**: cómo el kernel virtualiza la RAM, qué hace `malloc` por debajo, cómo mapear archivos directamente en tu espacio de direcciones, cómo compartir memoria entre procesos, y cómo construir tu propio allocator cuando `malloc` no es suficiente.

---

## Mapa del Bloque

```
Tema 1: Memoria virtual     →  Páginas, MMU, page faults, RSS vs VSZ
Tema 2: malloc/realloc/free →  Allocator de glibc, crecimiento amortizado
Tema 3: mmap                →  Archivos como memoria, MAP_SHARED vs MAP_PRIVATE
Tema 4: Shared memory POSIX →  shm_open, comunicación entre procesos
Tema 5: Resource limits     →  getrlimit/setrlimit, ulimit, cgroups
Tema 6: Pool allocators     →  Arena/bump allocator, cuándo construir tu propio
Tema 7: /proc y minitop     →  Observabilidad de memoria del sistema
```

---

## Tema 1 — Memoria virtual: lo que tu programa cree que tiene

### El mapa de memoria completo

Cuando tu proceso corre, el kernel le presenta un espacio de direcciones **virtual** de ~128 TB (en x86-64). Solo una fracción está realmente mapeada:

```
0xFFFFFFFFFFFF ┌─────────────────────────────┐
               │  Kernel space (inaccesible) │
0x7FFF...      ├─────────────────────────────┤
               │  Stack           ↓ crece     │  Variables locales, frames
               │                              │  Tamaño típico: 8 MB
               │                              │
               │  (espacio libre enorme)      │
               │                              │
               │  Mapeos mmap     ↕           │  Librerías .so, archivos mapeados,
               │                              │  bloques grandes de malloc
               │                              │
               │  Heap            ↑ crece     │  malloc/calloc/realloc
               ├─────────────────────────────┤
               │  BSS                         │  Globales no inicializadas (cero)
               │  Data                        │  Globales inicializadas
               │  Read-only data              │  Literales de string, const
               │  Text (código)               │  Instrucciones del programa
0x400000       └─────────────────────────────┘
```

### Páginas, MMU y page faults

La unidad mínima de memoria que el kernel maneja es una **página** (típicamente 4 KB en x86-64). La **MMU** (Memory Management Unit) del CPU traduce direcciones virtuales a físicas:

```
Tu programa: ptr = 0x7FFE1234
                    │
                    ▼
               Page Table (kernel)
               ┌──────────────────────────────┐
               │ Virtual 0x7FFE1000 → Phys 0x1A3000 │  ← presente en RAM
               │ Virtual 0x7FFE2000 → (no mapeada)  │  ← page fault!
               └──────────────────────────────┘
```

| Tipo de page fault | Causa | Resultado |
|--------------------|-------|-----------|
| **Minor** | Página asignada pero no cargada (ej: primera escritura en `malloc` grande) | Kernel asigna página física, continúa |
| **Major** | Página en disco (swap o archivo mmap) | Kernel lee de disco, puede ser lento |
| **Invalid** | Dirección no mapeada o violación de permisos | `SIGSEGV` → crash |

### RSS vs VSZ: entender lo que reporta `top`

| Métrica | Qué mide | Ejemplo |
|---------|----------|---------|
| **VSZ** (Virtual Size) | Todo el espacio virtual reservado | `malloc(1GB)` sin escribir → VSZ sube, RSS no |
| **RSS** (Resident Set Size) | Páginas físicamente en RAM | Solo sube cuando realmente accedes a la memoria |

```c
// Esto reserva 1 GB de espacio virtual pero no usa casi nada de RAM:
char *big = malloc(1024 * 1024 * 1024);
// VSZ ≈ 1 GB, RSS ≈ unos pocos KB

// Ahora sí usamos RAM real:
memset(big, 0, 1024 * 1024 * 1024);
// VSZ ≈ 1 GB, RSS ≈ 1 GB
```

> [!NOTE]
> **Overcommit:** Por defecto, Linux permite `malloc` de más memoria de la que existe físicamente. El kernel apuesta a que no todos la usarán simultáneamente. Si pierde la apuesta, el **OOM Killer** mata procesos. Puedes configurar esto vía `/proc/sys/vm/overcommit_memory`.

---

## Tema 2 — `malloc`, `realloc`, `free` y el allocator de glibc

### Lo que `malloc` hace por debajo

`malloc` no es una syscall — es una función de la librería C (glibc) que gestiona un **pool de memoria**:

```
Tu programa                     glibc allocator                    Kernel
┌─────────┐                     ┌──────────────────┐              ┌────────┐
│malloc(64)│ ──────────────────▶│ Arena interna     │              │        │
│          │  retorna puntero   │ ┌────┬────┬────┐  │  si agotada  │ brk()  │
│free(ptr) │ ◀─────────────────▶│ │used│free│used│  │ ───────────▶│ mmap() │
│          │                    │ └────┴────┴────┘  │              │        │
└─────────┘                     └──────────────────┘              └────────┘
```

- Para bloques **pequeños** (< ~128 KB): glibc los gestiona en arenas internas, usando `brk`/`sbrk` para extender el heap.
- Para bloques **grandes** (>= ~128 KB): glibc usa `mmap` directo. Este umbral es configurable con `mallopt(M_MMAP_THRESHOLD, ...)`.
- `free` **no siempre devuelve memoria al SO**. glibc puede retener bloques liberados para reutilizarlos rápido.

### `realloc`: la trampa del puntero perdido

```c
// ❌ INCORRECTO — si realloc falla, pierdes ptr:
ptr = realloc(ptr, new_size);
// Si falla: ptr = NULL, la memoria original se pierde → LEAK

// ✅ CORRECTO — preservar el puntero original:
void *tmp = realloc(ptr, new_size);
if (!tmp) {
    perror("realloc");
    // ptr sigue siendo válido con el contenido original
    // decidir: ¿abortar? ¿seguir con lo que tenemos?
} else {
    ptr = tmp;
}
```

### Array dinámico con crecimiento amortizado

```c
typedef struct {
    int   *data;
    size_t size;     // elementos en uso
    size_t capacity; // espacio reservado
} dynarray_t;

dynarray_t *dynarray_create(size_t initial_cap) {
    dynarray_t *a = malloc(sizeof(*a));
    if (!a) return NULL;
    a->data = malloc(initial_cap * sizeof(int));
    if (!a->data) { free(a); return NULL; }
    a->size = 0;
    a->capacity = initial_cap;
    return a;
}

int dynarray_push(dynarray_t *a, int value) {
    if (a->size == a->capacity) {
        size_t new_cap = a->capacity * 2;  // factor 2x
        if (new_cap < a->capacity) return -1;  // overflow check

        int *tmp = realloc(a->data, new_cap * sizeof(int));
        if (!tmp) return -1;
        a->data = tmp;
        a->capacity = new_cap;
    }
    a->data[a->size++] = value;
    return 0;
}
```

**¿Por qué factor 2x?** Cada vez que creces x2, copias todos los elementos existentes. Pero como el intervalo entre copias se duplica cada vez, el costo amortizado por inserción es **O(1)**. Con factor 1.5x también funciona (menos desperdicio, más copias).

### Integer overflow en cálculos de tamaño

```c
// ❌ PELIGROSO:
size_t total = nmemb * size;  // puede desbordar silenciosamente
void *p = malloc(total);      // asigna mucho menos de lo esperado

// ✅ SEGURO (lo que hace calloc internamente):
if (size != 0 && nmemb > SIZE_MAX / size) {
    // overflow detectado
    return NULL;
}
void *p = malloc(nmemb * size);
```

> [!CAUTION]
> `calloc(nmemb, size)` hace esta comprobación internamente. Si multiplicas tú mismo, debes verificar overflow.

---

## Tema 3 — `mmap`: archivos como memoria

### La idea

`mmap` mapea un archivo (o memoria anónima) directamente en el espacio de direcciones de tu proceso. En vez de `read(fd, buf, n)`, simplemente accedes a un puntero:

```c
#include <sys/mman.h>

int fd = open("data.bin", O_RDONLY);
struct stat st;
fstat(fd, &st);

// Mapear todo el archivo como lectura
void *ptr = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
if (ptr == MAP_FAILED) { perror("mmap"); close(fd); return -1; }
close(fd);  // el mapping sobrevive al close del FD

// Ahora puedes acceder al contenido del archivo como un array:
const char *data = ptr;
for (off_t i = 0; i < st.st_size; i++) {
    if (data[i] == '\n') line_count++;
}

munmap(ptr, st.st_size);  // liberar el mapping
```

### `MAP_SHARED` vs `MAP_PRIVATE`

```
MAP_PRIVATE (copy-on-write):
  Proceso ──▶ [copia privada] ──✗──▶ archivo original
  Tus cambios NO se escriben al archivo.
  Otros procesos NO ven tus cambios.

MAP_SHARED:
  Proceso A ──▶ ┌───────────┐ ──────▶ archivo en disco
  Proceso B ──▶ │ misma     │         (eventualmente)
                │ memoria   │
                └───────────┘
  Cambios de A son visibles para B (y viceversa).
  Con msync(), se persisten al disco.
```

| Flag | Cambios al archivo | Visible a otros procesos | Uso principal |
|------|-------------------|--------------------------|---------------|
| `MAP_PRIVATE` | No | No | Lectura de archivos grandes, parseo |
| `MAP_SHARED` | Sí (con `msync`) | Sí | IPC, bases de datos, archivos de estado |

### `mmap` anónimo: memoria sin archivo

```c
// Obtener 1 MB de memoria alineada a páginas, inicializada a cero:
void *mem = mmap(NULL, 1024*1024,
                 PROT_READ | PROT_WRITE,
                 MAP_PRIVATE | MAP_ANONYMOUS,
                 -1, 0);    // fd=-1 cuando es anónimo
```

Esto es lo que glibc usa internamente para bloques grandes de `malloc`.

### Errores y edge cases

| Situación | Resultado |
|-----------|-----------|
| `mmap` de archivo vacío (size=0) | Falla con `EINVAL` |
| Leer más allá del final del mapping | `SIGBUS` o `SIGSEGV` |
| Truncar archivo por debajo del mapping | `SIGBUS` al acceder a las páginas que ya no existen |
| Olvidar `munmap` | Fuga de VMAs (Virtual Memory Areas), leak en procesos largos |

### `mmap` vs `read`: ¿cuándo usar cada uno?

| Criterio | `mmap` gana | `read`/`write` gana |
|----------|------------|---------------------|
| Acceso aleatorio a archivo grande | ✓ (el kernel pagina bajo demanda) | ✗ (necesitas `lseek` + `read` por cada acceso) |
| Lectura secuencial completa | Empate o peor (TLB misses) | ✓ (más predecible, readahead funciona bien) |
| Compartir entre procesos | ✓ (`MAP_SHARED`) | ✗ (necesitas shm o pipe) |
| Archivos pequeños (< 64 KB) | ✗ (overhead del mapping) | ✓ |

---

## Tema 4 — Memoria compartida POSIX

### El flujo completo

```c
#include <sys/mman.h>
#include <fcntl.h>

// === Proceso 1: crear y escribir ===
int fd = shm_open("/mi_shm", O_CREAT | O_RDWR, 0666);
ftruncate(fd, 4096);  // establecer tamaño

int *shared = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
close(fd);

shared[0] = 42;       // escribir dato
shared[1] = 100;

// === Proceso 2 (independiente): abrir y leer ===
int fd = shm_open("/mi_shm", O_RDONLY, 0);
int *shared = mmap(NULL, 4096, PROT_READ, MAP_SHARED, fd, 0);
close(fd);

printf("Valor: %d\n", shared[0]);  // lee 42

// === Limpieza (en alguno de los procesos) ===
munmap(shared, 4096);
shm_unlink("/mi_shm");   // eliminar el objeto del namespace
```

> [!WARNING]
> **`shm_unlink` no destruye el mapping** — solo elimina el nombre de `/dev/shm`. Los procesos que ya tienen el mapping abierto siguen usándolo. Pero si olivas `shm_unlink`, el objeto persiste en `/dev/shm` hasta el reboot.

### Sincronización: lo que shared memory NO te da

Shared memory da **bytes compartidos**, no orden ni exclusión mutua. Si dos procesos escriben `shared[0]` simultáneamente, el resultado es una **race condition** idéntica a la de threads:

```c
// PELIGRO: sin sincronización
// Proceso A: shared[0]++;
// Proceso B: shared[0]++;
// Resultado esperado: +2. Resultado real: puede ser +1 (race condition)
```

Para sincronizar procesos sobre shared memory, necesitas:

| Mecanismo | API | Bloque donde se cubre |
|-----------|-----|----------------------|
| Semáforos POSIX | `sem_open`, `sem_wait`, `sem_post` | Bloque 5 |
| Mutexes compartidos | `pthread_mutex` con `PTHREAD_PROCESS_SHARED` | Bloque 5 |
| Atómicos | `stdatomic.h` (`_Atomic`) | Bloque 5 |

### Compilación

Los objetos de shared memory POSIX requieren linkar con `-lrt` en algunos sistemas:

```bash
gcc -o shm_demo shm_demo.c -lrt
```

---

## Tema 5 — Resource limits: `getrlimit`/`setrlimit`

### Soft limits vs Hard limits

```c
#include <sys/resource.h>

struct rlimit rl;
getrlimit(RLIMIT_NOFILE, &rl);
printf("Soft: %llu, Hard: %llu\n",
       (unsigned long long)rl.rlim_cur,
       (unsigned long long)rl.rlim_max);
```

| Tipo | Quién puede modificarlo | Qué pasa al excederlo |
|------|------------------------|----------------------|
| **Soft** (`rlim_cur`) | Cualquier proceso (dentro del rango) | El efecto depende del recurso |
| **Hard** (`rlim_max`) | Solo root puede **subir** | Define el techo del soft |

### Límites relevantes

| Recurso | Qué limita | Al exceder |
|---------|-----------|------------|
| `RLIMIT_NOFILE` | Máx FDs abiertos | `open` retorna `-1` `EMFILE` |
| `RLIMIT_AS` | Espacio virtual total | `mmap`/`malloc` fallan |
| `RLIMIT_STACK` | Tamaño del stack | `SIGSEGV` (stack overflow) |
| `RLIMIT_CORE` | Tamaño de core dumps | Core truncado o no generado |
| `RLIMIT_NPROC` | Máx procesos del usuario | `fork` falla con `EAGAIN` |

### Ejemplo: limitar memoria de un proceso

```c
// Limitar a 64 MB de memoria virtual
struct rlimit rl;
rl.rlim_cur = 64 * 1024 * 1024;
rl.rlim_max = 64 * 1024 * 1024;
if (setrlimit(RLIMIT_AS, &rl) == -1) {
    perror("setrlimit");
}

// Ahora malloc de 128 MB fallará:
void *p = malloc(128 * 1024 * 1024);
if (!p) {
    printf("malloc falló (como esperado)\n");
}
```

### Limits en Docker

En contenedores, hay **dos capas** de límites:

```
1. ulimit / setrlimit  →  por proceso (los de siempre)
2. cgroups             →  por contenedor (memory.max, cpu.max)
```

```bash
# Ver limits dentro del contenedor:
cat /proc/self/limits

# Docker flags:
docker run --ulimit nofile=1024:4096 ...
docker run --memory=256m ...
```

> [!NOTE]
> Si tu proceso muere con `SIGKILL` sin explicación dentro de Docker, probablemente fue el **OOM Killer** activado por el memory limit del cgroup (`docker run --memory=256m`). Revisa `dmesg` en el host.

---

## Tema 6 — Pool y arena allocators

### Cuándo `malloc` no es suficiente

| Problema | Ejemplo | Solución |
|----------|---------|----------|
| Muchas allocaciones pequeñas (overhead + fragmentación) | Parser que crea miles de nodos AST | Arena allocator |
| Latencia impredecible por lock contention en glibc | Sistema de tiempo real | Pool pre-allocado |
| Lifetime uniforme (se libera todo junto) | Procesamiento de un request HTTP | Arena por request |

### Arena / Bump allocator

La idea más simple posible: un bloque grande con un puntero que avanza:

```c
typedef struct {
    char   *base;      // inicio del bloque
    size_t  capacity;  // tamaño total
    size_t  offset;    // siguiente byte libre
} arena_t;

arena_t *arena_create(size_t capacity) {
    arena_t *a = malloc(sizeof(*a));
    if (!a) return NULL;
    a->base = malloc(capacity);
    if (!a->base) { free(a); return NULL; }
    a->capacity = capacity;
    a->offset = 0;
    return a;
}

void *arena_alloc(arena_t *a, size_t size) {
    // Alinear a 8 bytes para cualquier tipo
    size_t aligned = (size + 7) & ~(size_t)7;
    if (a->offset + aligned > a->capacity) return NULL;  // lleno

    void *ptr = a->base + a->offset;
    a->offset += aligned;
    return ptr;
}

void arena_reset(arena_t *a) {
    a->offset = 0;  // "liberar" todo — O(1), sin free individual
}

void arena_destroy(arena_t *a) {
    free(a->base);
    free(a);
}
```

**Ventajas:**
- Allocación O(1) (solo incrementar un puntero).
- Excelente cache locality (todo contiguo).
- Liberación O(1) (reset del offset).
- Cero fragmentación.

**Desventaja:**
- No puedes liberar objetos individuales. Todo vive hasta el `reset` o `destroy`.

### Pool allocator por tamaño fijo

Para cuando necesitas `alloc` y `free` individuales pero todos los objetos tienen el mismo tamaño:

```c
typedef struct pool_chunk {
    struct pool_chunk *next;   // free list: siguiente chunk libre
} pool_chunk_t;

typedef struct {
    char         *base;
    size_t        chunk_size;
    size_t        count;
    pool_chunk_t *free_list;   // lista enlazada de chunks libres
} pool_t;

pool_t *pool_create(size_t chunk_size, size_t count) {
    // chunk_size debe ser >= sizeof(pool_chunk_t)
    if (chunk_size < sizeof(pool_chunk_t))
        chunk_size = sizeof(pool_chunk_t);

    pool_t *p = malloc(sizeof(*p));
    p->base = malloc(chunk_size * count);
    p->chunk_size = chunk_size;
    p->count = count;

    // Construir free list enlazando todos los chunks
    p->free_list = NULL;
    for (size_t i = 0; i < count; i++) {
        pool_chunk_t *chunk = (pool_chunk_t *)(p->base + i * chunk_size);
        chunk->next = p->free_list;
        p->free_list = chunk;
    }
    return p;
}

void *pool_alloc(pool_t *p) {
    if (!p->free_list) return NULL;  // pool agotado
    pool_chunk_t *chunk = p->free_list;
    p->free_list = chunk->next;
    return chunk;
}

void pool_free(pool_t *p, void *ptr) {
    pool_chunk_t *chunk = ptr;
    chunk->next = p->free_list;
    p->free_list = chunk;
}
```

Alloc y free son **O(1)** sin fragments.

---

## Tema 7 — `/proc` y el proyecto `minitop`

### `/proc`: el filesystem virtual del kernel

`/proc` no existe en disco. Es generado dinámicamente por el kernel para exponer estado del sistema:

```
/proc/
├── meminfo          ← memoria global del sistema
├── loadavg          ← carga promedio
├── cpuinfo          ← info de CPUs
├── stat             ← estadísticas de CPU globales
├── 1/               ← proceso PID 1
│   ├── stat         ← una línea con ~52 campos
│   ├── status       ← formato clave: valor legible
│   ├── cmdline      ← comando que inició el proceso
│   └── maps         ← mapeos de memoria
├── 1234/            ← proceso PID 1234
│   └── ...
└── self/            ← symlink al PID del lector
```

### Parsear `/proc/meminfo`

```
MemTotal:       16384000 kB
MemFree:         2048000 kB
MemAvailable:    8192000 kB
Buffers:          512000 kB
Cached:          4096000 kB
...
```

```c
FILE *f = fopen("/proc/meminfo", "r");
char line[256];
unsigned long mem_total = 0, mem_free = 0, mem_avail = 0;

while (fgets(line, sizeof(line), f)) {
    sscanf(line, "MemTotal: %lu kB", &mem_total);
    sscanf(line, "MemFree: %lu kB", &mem_free);
    sscanf(line, "MemAvailable: %lu kB", &mem_avail);
}
fclose(f);

printf("Total: %lu MB, Used: %lu MB\n",
       mem_total / 1024, (mem_total - mem_avail) / 1024);
```

### Parsear `/proc/[pid]/stat`

Este archivo es una sola línea con ~52 campos separados por espacios. La dificultad: el campo 2 (`comm`) puede contener espacios si el nombre del proceso los tiene:

```
1234 (mi proceso) S 1233 1234 1234 0 -1 4194304 ...
```

```c
// Parsing seguro: buscar el último ')' para evitar el problema del comm con espacios
char *start = strchr(line, '(');
char *end = strrchr(line, ')');
if (!start || !end) return -1;

// Extraer comm
size_t comm_len = end - start - 1;
// Parsear campos después del ')'
char state;
int ppid;
sscanf(end + 2, "%c %d ...", &state, &ppid, ...);
```

### Escanear todos los procesos

```c
DIR *proc = opendir("/proc");
struct dirent *ent;

while ((ent = readdir(proc)) != NULL) {
    // Filtrar solo directorios numéricos (PIDs)
    if (ent->d_type != DT_DIR) continue;

    char *endptr;
    long pid = strtol(ent->d_name, &endptr, 10);
    if (*endptr != '\0') continue;  // no es un número

    // Leer /proc/<pid>/stat
    char path[64];
    snprintf(path, sizeof(path), "/proc/%ld/stat", pid);
    // ... parsear ...
}
closedir(proc);
```

> [!TIP]
> **Los procesos pueden desaparecer entre `readdir` y `open`.** No es un error — el proceso simplemente terminó. Maneja `ENOENT` silenciosamente y continúa.

### Refresh periódico con ANSI escape codes

```c
// Limpiar pantalla y posicionar cursor al inicio:
printf("\033[2J\033[H");

// Imprimir tabla de procesos:
printf("  PID  STATE  RSS (KB)  COMMAND\n");
for (int i = 0; i < n_procs; i++) {
    printf("%5d  %c      %8lu  %s\n",
           procs[i].pid, procs[i].state,
           procs[i].rss, procs[i].comm);
}

// Esperar N segundos:
sleep(refresh_interval);
```

---

## Checklist de salida del Bloque 04

- [ ] Implementar un array dinámico con `realloc` y crecimiento x2, sin leaks
- [ ] Mapear un archivo con `mmap`, buscar un patrón, y `munmap` correctamente
- [ ] Modificar un archivo via `MAP_SHARED` + `msync` y verificar persistencia
- [ ] Comunicar dos procesos via `shm_open` + `mmap` compartido con cleanup correcto
- [ ] Consultar y modificar `RLIMIT_NOFILE` y observar que `open` falla al excederlo
- [ ] Implementar un arena allocator con `alloc` O(1) y `reset`
- [ ] Leer `/proc/meminfo` y `/proc/[pid]/stat` para construir un monitor básico

---

## Referencias

| Recurso | Comando |
|---------|---------|
| Allocación | `man 3 malloc`, `man 3 calloc`, `man 3 realloc`, `man 3 free` |
| Memory mapping | `man 2 mmap`, `man 2 munmap`, `man 2 msync` |
| Shared memory | `man 3 shm_open`, `man 3 shm_unlink`, `man 2 ftruncate` |
| Resource limits | `man 2 getrlimit`, `man 2 setrlimit`, `man 1 ulimit` |
| Proc filesystem | `man 5 proc` |
| Overcommit | `man 5 proc` (buscar `overcommit_memory`) |
