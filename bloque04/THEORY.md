# THEORY.md — Bloque 04: Memoria, mapeos y límites en Linux

En este bloque pasas de "usar memoria" a "diseñar comportamiento de memoria".
La diferencia entre un programa frágil y uno robusto suele estar aquí: asignación dinámica, mapeos, recursos compartidos y límites operativos.

## Alcance del bloque

Debes dominar:

1. Modelo de memoria de proceso y allocator de `glibc`.
2. Diseño de estructuras dinámicas con `malloc/realloc/free`.
3. `mmap` para archivos y memoria anónima.
4. Memoria compartida POSIX (`shm_open` + `mmap`).
5. Límites por proceso (`getrlimit/setrlimit`).
6. Diseño de allocators simples (pool/arena).
7. Lectura de métricas reales de `/proc` para observabilidad (`minitop`).

Material previo del bloque quedó en `bloque04/ej_legacy/`.

---

## 1) Mapa de memoria de un proceso Linux

## 1.1 Segmentos conceptuales

Un proceso típico en Linux tiene regiones:

1. `text` (código)
2. `rodata` (constantes)
3. `data`/`bss` (globales)
4. heap (asignación dinámica)
5. stack (frames de funciones)
6. mapeos extra (`mmap`) de librerías/archivos/anon

No todas las regiones son contiguas ni estáticas; el kernel gestiona VMAs (Virtual Memory Areas).

## 1.2 Memoria virtual y páginas

La memoria que ves en C es virtual.
El hardware (MMU) traduce páginas virtuales a físicas con tablas de páginas.

Consecuencias:

1. puedes mapear archivos gigantes sin cargar todo a RAM inmediata.
2. accesos disparan page faults cuando falta página presente.
3. protección por página (`R/W/X`) se aplica por hardware + kernel.

## 1.3 RSS, VSZ y confusiones comunes

- VSZ: tamaño de espacio virtual reservado.
- RSS: páginas realmente residentes en RAM.

Muchos programas "parecen" grandes por VSZ, pero usan poco RSS real.

---

## 2) `malloc`, `realloc`, `free` y allocator de glibc

## 2.1 Contratos básicos

```c
void *malloc(size_t n);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t n);
void free(void *ptr);
```

Reglas:

1. `malloc/calloc/realloc` pueden devolver `NULL`.
2. `free(NULL)` es seguro (no-op).
3. usar memoria tras `free` = UB (use-after-free).
4. liberar dos veces = UB (double free).

## 2.2 `calloc` vs `malloc`

`calloc` inicializa a cero.
No lo uses por "costumbre" sin necesidad; úsalo cuando el cero semánticamente importa.

## 2.3 `realloc` sin perder puntero

Patrón correcto:

```c
void *tmp = realloc(ptr, new_size);
if (!tmp) {
    /* ptr original sigue válido */
    // manejar error
} else {
    ptr = tmp;
}
```

Nunca hagas `ptr = realloc(ptr, ...)` sin controlar error si quieres preservar memoria previa.

## 2.4 Estrategia de crecimiento amortizado

Para arrays dinámicos, crecer x2 es patrón estándar:

1. reduce número de realocaciones
2. inserción amortizada O(1)

Costo de copiar en `realloc` aparece solo en ciertos crecimientos.

## 2.5 `glibc` y relación con kernel

El allocator de `glibc` gestiona heaps internos y puede pedir memoria al kernel vía:

1. `brk/sbrk` (histórico)
2. `mmap` para bloques grandes

`free` no siempre devuelve inmediatamente memoria al SO; puede retenerla para reutilización rápida.

## 2.6 Fragmentación

Tipos:

1. interna: sobre-asignación dentro de bloques.
2. externa: huecos no reutilizables eficientemente.

Pool allocators y arenas existen para mitigar este costo en patrones específicos.

---

## 3) Integridad de memoria y diagnóstico

## 3.1 Bugs clásicos del bloque

1. leak
2. use-after-free
3. out-of-bounds
4. double free
5. integer overflow en cálculos de tamaño

## 3.2 Herramientas recomendadas

1. ASan/UBSan en debug (`-fsanitize=address,undefined`)
2. Valgrind para análisis de fugas/accesos

## 3.3 Patrones defensivos

1. inicializar punteros a `NULL`
2. después de `free`, opcionalmente setear puntero a `NULL`
3. centralizar cleanup en una ruta
4. validar tamaños antes de multiplicar (`nmemb * size`)

---

## 4) `mmap`: archivos como memoria

## 4.1 Qué hace `mmap`

```c
void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off);
```

Mapea una región virtual a:

1. archivo (`fd` válido)
2. memoria anónima (`MAP_ANONYMOUS`, fd = -1)

## 4.2 `MAP_SHARED` vs `MAP_PRIVATE`

- `MAP_SHARED`: cambios visibles y potencialmente persistibles en archivo.
- `MAP_PRIVATE`: copy-on-write privado; no persiste en archivo.

## 4.3 Protección y fallos

`PROT_READ`, `PROT_WRITE`, `PROT_EXEC` controlan permisos.
Escribir en mapping solo lectura puede causar `SIGSEGV`.

## 4.4 `msync` y persistencia

En `MAP_SHARED`, `msync` fuerza sincronización explícita:

```c
msync(ptr, len, MS_SYNC);
```

Sin `msync`, el kernel puede diferir flush según políticas de page cache.

## 4.5 `munmap` obligatorio

Toda región mapeada debe liberarse con:

```c
munmap(ptr, len);
```

No hacerlo en procesos largos produce crecimiento de VMAs/consumo.

## 4.6 Casos borde

1. mapear archivo vacío (`len=0`) falla.
2. acceder fuera del rango mapeado -> `SIGBUS`/`SIGSEGV`.
3. truncar archivo por debajo del mapping activo puede provocar `SIGBUS`.

## 4.7 Cuándo usar `mmap`

Útil en:

1. acceso aleatorio a archivos grandes
2. sharing entre procesos
3. parsers de estructuras grandes de solo lectura

No siempre supera a `read/write` en flujos secuenciales simples.

---

## 5) Memoria compartida POSIX (`shm_open`)

## 5.1 Flujo correcto

1. `shm_open(name, O_CREAT|O_RDWR, mode)`
2. `ftruncate(fd, size)`
3. `mmap(..., MAP_SHARED, fd, 0)`
4. `fork` o procesos independientes mapean mismo objeto
5. comunicación sobre misma región
6. `munmap`, `close`, `shm_unlink(name)`

## 5.2 Nombre y namespace

`name` debe empezar con `/` (ej. `/mi_shm`).
Se materializa en namespace de memoria compartida (habitualmente visible bajo `/dev/shm`).

## 5.3 Sincronización: lo que NO resuelve shm

Shared memory comparte bytes, no sincronización.
Si hay escritores concurrentes, necesitas protocolo:

1. mutex/semaphore/shared atomics
2. flags de estado
3. orden de memoria claro

## 5.4 Relación con `fork`

Tras `fork`, ambos procesos ya ven mapping compartido.
Para procesos no emparentados, ambos deben abrir/mapear el mismo `name`.

## 5.5 Limpieza y leaks de objetos shm

Olvidar `shm_unlink` deja objetos residuales en el sistema.
En entornos de laboratorio puede saturar `/dev/shm` con objetos huérfanos.

---

## 6) Límites de recursos (`getrlimit/setrlimit`)

## 6.1 Concepto soft/hard

`struct rlimit`:

1. `rlim_cur` (soft)
2. `rlim_max` (hard)

Proceso no privilegiado puede bajar límites; subir hard limit requiere privilegio.

## 6.2 Límites relevantes para este bloque

1. `RLIMIT_AS` memoria virtual total
2. `RLIMIT_DATA` segmento de datos/heap (dependiente plataforma)
3. `RLIMIT_NOFILE` max FDs abiertos
4. `RLIMIT_CORE` tamaño de core dumps

## 6.3 Uso típico

```c
struct rlimit r;
getrlimit(RLIMIT_NOFILE, &r);
```

Modificar:

```c
r.rlim_cur = nuevo;
setrlimit(RLIMIT_NOFILE, &r);
```

## 6.4 Efecto real sobre `malloc`

Si limitas `RLIMIT_AS` agresivamente, `malloc` puede fallar antes de consumir físicamente mucha RAM.
No siempre verás OOM killer; a menudo verás `NULL` limpio si controlas errores.

## 6.5 `ulimit` y contenedores

Dentro de contenedores, límites pueden venir de:

1. `ulimit` del proceso
2. cgroups/mem limits del runtime

Interpretar fallos de memoria requiere mirar ambos niveles.

---

## 7) Pool/Arena allocators

## 7.1 Motivación

Muchos objetos pequeños + `malloc/free` frecuente puede costar por:

1. metadatos
2. locks internos allocator
3. fragmentación

## 7.2 Arena lineal (bump allocator)

Modelo:

1. reservar bloque grande (`base`, `capacity`)
2. `offset` avanza linealmente por cada alloc
3. `free` individual no existe
4. liberación total al final (`free(base)`)

## 7.3 Ventajas

1. asignación O(1)
2. excelente localidad
3. implementación simple

## 7.4 Desventajas

1. no permite liberar objetos individuales
2. puede desperdiciar memoria por vida útil heterogénea
3. requiere diseño claro de ownership/lifetime

## 7.5 Estrategias híbridas

1. arenas por fase
2. pools por tamaño de objeto
3. fallback a `malloc` para casos raros

---

## 8) `/proc` y observabilidad de memoria (base de `minitop`)

## 8.1 `/proc` como interfaz del kernel

`/proc` exporta estado dinámico en formato texto/virtual FS.

## 8.2 Archivos clave

1. `/proc/meminfo`
2. `/proc/loadavg`
3. `/proc/<pid>/stat`
4. `/proc/<pid>/status`

## 8.3 Parsing robusto

Reglas:

1. no asumir formato fijo más allá de campos documentados
2. validar retornos de `fgets/sscanf`
3. tolerar campos faltantes sin crashear

## 8.4 Escaneo de PIDs

`/proc` contiene entradas no numéricas (`sys`, `net`, etc.).
Filtra con `isdigit` en nombre completo.

## 8.5 Métricas útiles para mini-top

1. memoria usada global (a partir de `MemTotal`, `MemFree`, `Buffers`, `Cached`)
2. estado de proceso (`R/S/D/Z/...`)
3. RSS por proceso

---

## 9) Patrones de robustez del bloque

## 9.1 Chequeo de retornos siempre

`malloc`, `realloc`, `open`, `mmap`, `msync`, `munmap`, `shm_open`, `ftruncate`, `setrlimit`.
Todo puede fallar.

## 9.2 Cleanup idempotente

Mantén punteros/FDs inicializados (`NULL`/`-1`) y cleanup único al final.

## 9.3 Seguridad de tamaños

Antes de reservar/copiar:

1. validar overflow en multiplicaciones
2. validar límites de usuario
3. no confiar en tamaños de archivos sin sanity checks

## 9.4 Señales y memoria

Si combinas señales con estructuras de memoria compartida, define protocolo explícito; evita lógica compleja dentro del handler.

## 9.5 Portabilidad mínima

Algunos detalles (`RLIMIT_DATA`, flags específicos, `-lrt` histórico) varían por distro/kernel/libc.
Mantén código defensivo y tests explícitos.

---

## 10) Criterios de dominio del Bloque 04

Debes poder:

1. diseñar un vector dinámico con crecimiento amortizado sin leaks.
2. mapear un archivo con `mmap`, modificarlo y persistir cambios.
3. comunicar padre/hijo por shared memory con cleanup correcto.
4. aplicar y observar límites con `getrlimit/setrlimit`.
5. implementar un pool allocator lineal y explicar sus trade-offs.
6. extraer métricas útiles de `/proc` para un monitor básico.

Si fallas en estos puntos, los bloques de concurrencia/redes/seguridad heredan bugs complejos.

---

## 11) Referencias técnicas recomendadas

1. `man 3 malloc`, `man 3 realloc`, `man 3 free`
2. `man 2 mmap`, `man 2 munmap`, `man 2 msync`
3. `man 3 shm_open`, `man 3 shm_unlink`, `man 2 ftruncate`
4. `man 2 getrlimit`, `man 2 setrlimit`, `man 1 ulimit`
5. `man 5 proc`, `man 5 procfs`
6. `man 2 open`, `man 2 stat`

---

## Práctica del bloque

Los ejercicios nuevos del bloque (10 resueltos pedagógicos + 3 complejos) están en:

- `bloque04/EJERCICIOS.md`
- `bloque04/practica/`
