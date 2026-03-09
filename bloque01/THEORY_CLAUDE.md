# THEORY_CLAUDE.md — Bloque 01: Fundamentos sólidos de C en Linux

> Este bloque no enseña "C de academia". Enseña a construir utilidades Unix que manejen errores, datos malformados y límites de memoria como lo hacen `grep`, `cat` y `find`. Todo lo que escribas de aquí en adelante — daemons, servidores, herramientas de red — depende de que estos fundamentos estén sólidos.

---

## Mapa del Bloque

```
Tema 1: CLI en Linux       →  argc/argv, getopt, convenciones de salida
Tema 2: Strings seguros    →  Buffers, null-termination, wrappers anti-overflow
Tema 3: Structs genéricos  →  Listas enlazadas con void *, ownership de memoria
Tema 4: Errores POSIX      →  errno, goto cleanup, macros de chequeo
Tema 5: Punteros y memoria →  memcpy/memmove/memset artesanales + benchmarking
Tema 6: Proyecto integrador →  miniecho y minicat: todo junto
```

---

## Tema 1 — Programas CLI: El contrato entre tu binario y el sistema

### Qué recibe tu programa al ejecutarse

Cuando el usuario escribe:

```bash
./mi_tool -v --output=resultado.txt datos.csv
```

El **shell** (no tu programa) hace todo esto antes de llamarte:
1. Tokeniza por espacios (respetando comillas).
2. Expande wildcards (`*.c` → lista de archivos).
3. Expande variables (`$HOME` → `/home/user`).
4. Construye el array `argv` y lo pasa al kernel con `execvp`.

Tu programa recibe tokens ya procesados:

```
argv[0] = "./mi_tool"
argv[1] = "-v"
argv[2] = "--output=resultado.txt"
argv[3] = "datos.csv"
argv[4] = NULL          ← siempre: argv[argc] == NULL
```

> [!NOTE]
> `argv[0]` **no siempre es confiable**. Si el programa fue invocado via `exec` con un nombre falso, contendrá lo que el caller haya puesto. Úsalo solo para mensajes de ayuda, nunca para lógica de seguridad.

### Las 3 reglas de salida de una utilidad Unix

| Regla | Cómo | Por qué |
|-------|------|---------|
| Datos van a `stdout` | `printf(...)` o `fprintf(stdout, ...)` | Permite pipes: `mi_tool | grep patron` |
| Errores van a `stderr` | `fprintf(stderr, ...)` | No contamina la salida de datos en el pipe |
| Código de salida coherente | `return EXIT_SUCCESS` o `return EXIT_FAILURE` | Permite: `mi_tool && echo "OK"` |

```bash
# El usuario espera poder hacer esto sin que errores se mezclen con datos:
./mi_tool --list > salida.txt 2> errores.log
```

> [!WARNING]
> Si imprimes un error con `printf()` en vez de `fprintf(stderr, ...)`, ese error viajará por el pipe al siguiente programa como si fuera un dato legítimo. Este bug es silencioso y destructivo.

### Parseo de opciones con `getopt_long`

Parsear `argv` a mano con `strcmp` es frágil: no maneja opciones combinadas (`-vn`), no detecta argumentos faltantes, no soporta `--`. La solución es `getopt`/`getopt_long`:

```c
#include <unistd.h>    // getopt
#include <getopt.h>    // getopt_long

static struct option long_opts[] = {
    {"verbose", no_argument,       NULL, 'v'},
    {"output",  required_argument, NULL, 'o'},
    {"count",   required_argument, NULL, 'n'},
    {"help",    no_argument,       NULL, 'h'},
    {0, 0, 0, 0}   // centinela obligatorio
};

int main(int argc, char *argv[]) {
    int verbose = 0;
    const char *output = NULL;
    long count = 1;

    int opt;
    while ((opt = getopt_long(argc, argv, "vo:n:h", long_opts, NULL)) != -1) {
        switch (opt) {
        case 'v': verbose = 1; break;
        case 'o': output = optarg; break;
        case 'n':
            count = parse_long(optarg, 1, 1000);  // ver abajo
            if (count < 0) return EXIT_FAILURE;
            break;
        case 'h':
            print_usage(stdout, argv[0]);
            return EXIT_SUCCESS;
        default:  // '?' — getopt ya imprimió el error
            print_usage(stderr, argv[0]);
            return EXIT_FAILURE;
        }
    }

    // Argumentos posicionales: argv[optind], argv[optind+1], ...
    for (int i = optind; i < argc; i++) {
        process_file(argv[i]);
    }
    return EXIT_SUCCESS;
}
```

### La `optstring` decodificada

```
"vo:n:h"
 │││││
 ││││└─ 'h' sin argumento
 │││└── 'n' requiere argumento (por el ':')
 ││└─── el ':' indica que la opción anterior requiere argumento
 │└──── 'o' requiere argumento
 └───── 'v' sin argumento
```

| Retorno de `getopt` | Significado |
|---------------------|-------------|
| `'v'`, `'o'`, etc. | Opción reconocida. Si lleva `:`, su valor está en `optarg` |
| `'?'` | Opción desconocida o argumento faltante |
| `-1` | No quedan más opciones |

Casos especiales:
- `--` en la línea de comandos marca fin de opciones. Todo lo que sigue son posicionales, incluso si empieza con `-`.
- Si `optstring` empieza con `:` (ej: `":vo:n:h"`), argumento faltante retorna `:` en vez de `?`, permitiendo mensajes de error diferenciados.

### Variables globales de `getopt`

| Variable | Propósito |
|----------|-----------|
| `optarg` | Valor del argumento de la opción actual |
| `optind` | Índice en `argv` del próximo elemento a procesar |
| `optopt` | La opción que causó error (en caso de `'?'`) |
| `opterr` | Si es `1` (default), getopt imprime errores automáticamente. Ponerlo a `0` para manejar errores tú mismo |

### Conversión numérica: por qué `atoi` es una trampa

`atoi("42abc")` retorna `42` silenciosamente. `atoi("9999999999999")` es UB por overflow. `atoi("")` retorna `0`, indistinguible de un `"0"` legítimo.

La alternativa robusta es `strtol`:

```c
static long parse_long(const char *str, long min, long max) {
    char *endptr;
    errno = 0;
    long val = strtol(str, &endptr, 10);

    if (str == endptr) {
        // No se consumió nada: "abc" → endptr == str
        fprintf(stderr, "Error: '%s' no es un número\n", str);
        return -1;
    }
    if (*endptr != '\0') {
        // Sobró texto: "42abc" → endptr apunta a 'a'
        fprintf(stderr, "Error: texto sobrante en '%s'\n", str);
        return -1;
    }
    if (errno == ERANGE || val < min || val > max) {
        fprintf(stderr, "Error: '%s' fuera de rango [%ld, %ld]\n", str, min, max);
        return -1;
    }
    return val;
}
```

### Función `print_usage`: el estándar

```c
static void print_usage(FILE *out, const char *prog) {
    fprintf(out, "Uso: %s [OPCIONES] [ARCHIVOS...]\n", prog);
    fprintf(out, "\nOpciones:\n");
    fprintf(out, "  -v, --verbose     Salida detallada\n");
    fprintf(out, "  -o, --output=FILE Archivo de salida\n");
    fprintf(out, "  -n, --count=N     Número de repeticiones (1-1000)\n");
    fprintf(out, "  -h, --help        Mostrar esta ayuda\n");
}
```

> [!IMPORTANT]
> Imprime a `stdout` cuando el usuario pide `--help` (éxito). Imprime a `stderr` cuando hay error de uso (flag inválida). Por eso `print_usage` recibe un `FILE *out`.

---

## Tema 2 — Strings en C: No son strings, son arrays de bytes con convención

### El modelo real

Un "string" en C no es un tipo. Es una **convención**: un array de `char` donde el primer byte con valor `0` (`'\0'`) marca el final.

```c
char s[] = "Hola";
// En memoria: {'H', 'o', 'l', 'a', '\0'}
// sizeof(s) == 5 (incluye el null)
// strlen(s) == 4 (no incluye el null)
```

Toda la librería estándar (`strlen`, `strcmp`, `printf("%s")`) **recorre byte a byte hasta encontrar `'\0'`**. Si no hay `'\0'`, sigue leyendo hasta un segfault o hasta encontrar un cero aleatorio en la memoria.

### El peligro fundamental: un `char *` no sabe su tamaño

```c
void copia(char *dst, const char *src) {
    strcpy(dst, src);  // ¿cuántos bytes caben en dst? nadie lo sabe.
}
```

Si `src` tiene 100 bytes y `dst` solo 10, `strcpy` escribe 90 bytes fuera de los límites del buffer. Esto es un **buffer overflow**: el bug más explotado en la historia de la seguridad informática.

### La falsa seguridad de `strncpy`

```c
strncpy(dst, src, n);
```

`strncpy` fue diseñada en los años 70 para campos de longitud fija (como nombres de archivos en estructuras de disco), **no para strings generales**. Sus problemas:

1. **Si `strlen(src) >= n`: no escribe `'\0'`.** Tu "string" queda sin terminador.
2. **Si `strlen(src) < n`: rellena con ceros hasta `n`.** Desperdicio innecesario en buffers grandes.

> [!CAUTION]
> `strncpy(dst, src, sizeof(dst))` parece seguro pero no lo es. Si `src` es del mismo tamaño o mayor que `dst`, el resultado **no tiene null terminator**. `strlen(dst)` después de eso lee hasta encontrar un `'\0'` quién sabe dónde.

### Diseñando wrappers seguros: el contrato que importa

Los ejercicios del bloque piden implementar `safe_strcpy`, `safe_strcat`, `safe_snprintf`. El contrato correcto (inspirado en `strlcpy` de OpenBSD) es:

1. **Siempre recibir `dst_size`** (la capacidad total del buffer, incluyendo espacio para `'\0'`).
2. **Siempre null-terminar** (cuando `dst_size > 0`).
3. **Retornar la longitud que se necesitaría** para detectar truncamiento.

#### `safe_strcpy`

```c
// Retorna: strlen(src). Si retorno >= dst_size, hubo truncamiento.
size_t safe_strcpy(char *dst, size_t dst_size, const char *src) {
    size_t src_len = strlen(src);
    if (dst_size == 0) return src_len;  // nada que hacer

    size_t copy_len = (src_len < dst_size) ? src_len : dst_size - 1;
    memcpy(dst, src, copy_len);
    dst[copy_len] = '\0';
    return src_len;
}
```

Uso:

```c
char buf[16];
size_t needed = safe_strcpy(buf, sizeof(buf), user_input);
if (needed >= sizeof(buf)) {
    fprintf(stderr, "Advertencia: input truncado (%zu → %zu)\n", needed, sizeof(buf) - 1);
}
```

#### `safe_strcat`

```c
// Retorna: dst_len + src_len. Si retorno >= dst_size, hubo truncamiento.
size_t safe_strcat(char *dst, size_t dst_size, const char *src) {
    // Encontrar el final de dst sin salir de dst_size
    size_t dst_len = 0;
    while (dst_len < dst_size && dst[dst_len] != '\0') dst_len++;

    // Si dst no tiene '\0' dentro de dst_size, el buffer ya está corrupto
    if (dst_len == dst_size) return dst_size + strlen(src);

    size_t src_len = strlen(src);
    size_t avail = dst_size - dst_len - 1;  // espacio disponible para nuevo contenido
    size_t copy_len = (src_len < avail) ? src_len : avail;

    memcpy(dst + dst_len, src, copy_len);
    dst[dst_len + copy_len] = '\0';
    return dst_len + src_len;
}
```

#### `safe_snprintf` (wrapper variádico)

```c
#include <stdarg.h>

int safe_snprintf(char *dst, size_t n, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vsnprintf(dst, n, fmt, ap);
    va_end(ap);
    return r;
    // r < 0  → error de encoding
    // r >= n → truncamiento
    // r < n  → éxito completo
}
```

### Trampas comunes con buffers

| Trampa | Ejemplo | Problema |
|--------|---------|----------|
| `sizeof` en punteros | `void f(char *buf) { sizeof(buf); }` | Retorna 8 (tamaño del puntero), no del buffer |
| `sprintf` sin límite | `sprintf(buf, "user=%s", input)` | Sin límite de escritura → overflow |
| Olvidar el `'\0'` en el conteo | Buffer de 10 → solo 9 chars útiles | Off-by-one clásico |
| Asumir ASCII | `strlen("café")` | Con UTF-8, `é` ocupa 2 bytes → strlen retorna 5, no 4 |

### Testing mínimo para funciones de string

Si tus tests no cubren estos casos, no has testeado:

1. `dst_size = 0` — ¿se comporta sin crash?
2. `dst_size = 1` — ¿solo escribe `'\0'`?
3. `src = ""` — string vacío
4. `src` mucho más largo que `dst` — truncamiento
5. `dst` casi lleno antes de `safe_strcat` — solo quedan 1-2 bytes

---

## Tema 3 — Structs, `void *` y ownership de memoria

### Por qué una lista enlazada

No es por rendimiento (un array con `realloc` es casi siempre más rápido por cache locality). Es porque fuerza a dominar:

- Punteros a punteros (`head = &(node->next)`).
- Asignación/liberación por nodo individual.
- La pregunta crítica: **¿quién libera la data?**

### Layout de struct: padding y alineación

```c
struct ejemplo {
    char   a;    // 1 byte
    // 3 bytes de padding (el compilador alinea 'b' a 4 bytes)
    int    b;    // 4 bytes
    char   c;    // 1 byte
    // 3 bytes de padding (para que sizeof total sea múltiplo de 4)
};
// sizeof(struct ejemplo) == 12, no 6
```

> [!WARNING]
> **Nunca escribas un struct directamente a disco o red** con `fwrite(&s, sizeof(s), 1, f)`. El padding varía entre compiladores, arquitecturas y flags de compilación. Para serialización, escribe campo por campo en un formato definido.

### API de lista genérica

```c
typedef struct node {
    void        *data;
    struct node *next;
} node_t;

typedef struct {
    node_t *head;
    size_t  size;
} list_t;
```

Las operaciones mínimas:

```c
list_t *list_create(void);
int     list_push(list_t *list, void *data);     // 0 éxito, -1 error
void   *list_pop(list_t *list);                   // NULL si vacía
void    list_destroy(list_t *list, void (*free_fn)(void *));
```

### Implementación segura: qué puede fallar

**`list_push` — el orden importa:**

```c
int list_push(list_t *list, void *data) {
    node_t *node = malloc(sizeof(*node));
    if (!node) return -1;    // malloc falló: NO modificar la lista

    node->data = data;
    node->next = list->head; // primero enlazar al antiguo head
    list->head = node;       // luego actualizar head
    list->size++;
    return 0;
}
```

Si hicieras `list->head = node` antes de `node->next = list->head`, perderías la referencia al resto de la lista.

**`list_pop` — devolver data, liberar nodo:**

```c
void *list_pop(list_t *list) {
    if (!list->head) return NULL;   // lista vacía, no crash

    node_t *old = list->head;
    void *data = old->data;
    list->head = old->next;
    list->size--;
    free(old);                      // liberar el nodo, NO la data
    return data;                    // caller decide qué hacer con ella
}
```

### El dilema del ownership: ¿quién hace `free(data)`?

Esta es la decisión de diseño más importante de cualquier estructura de datos genérica en C:

| Modelo | `list_destroy` hace... | Ventaja | Peligro |
|--------|----------------------|---------|---------|
| **Lista dueña** | `free(data)` + `free(node)` | Simple. Todo se limpia automáticamente | Si `data` apunta a stack o a un literal, `free` crashea |
| **Lista no dueña** | Solo `free(node)` | Flexible. Almacena cualquier tipo | Si el caller olvida liberar data, leak |
| **Callback** | `free_fn(data)` + `free(node)` | Lo mejor de ambos mundos | Ligeramente más complejo de usar |

La solución profesional es el callback:

```c
void list_destroy(list_t *list, void (*free_fn)(void *)) {
    node_t *cur = list->head;
    while (cur) {
        node_t *next = cur->next;
        if (free_fn) free_fn(cur->data);
        free(cur);
        cur = next;
    }
    free(list);
}

// Uso con datos heap-allocated:
list_destroy(mi_lista, free);

// Uso con datos que no deben liberarse (stack, literales):
list_destroy(mi_lista, NULL);
```

### `void *`: reglas para no corromperte

- `void *` acepta cualquier dirección sin cast explícito (en C, no en C++).
- **Nunca desreferencies `void *` directamente.** Necesitas cast al tipo real.
- El cast debe coincidir con el tipo almacenado. `int *` → push como `void *` → pop y castear a `double *` = **undefined behavior**.

```c
// ERROR CLÁSICO: almacenar dirección de variable local
void bad_example(list_t *list) {
    int x = 42;
    list_push(list, &x);  // &x será inválido cuando bad_example retorne
}
// Después, pop devuelve un puntero a memoria de stack ya liberada → UB
```

**Solución:** Para tipos primitivos, allocar en heap:

```c
int *val = malloc(sizeof(int));
*val = 42;
list_push(list, val);
```

### Invariantes: lo que siempre debe cumplirse

Después de cualquier operación, estas condiciones deben ser verdad:

1. `list->size >= 0` siempre.
2. `list->size == 0` ⟺ `list->head == NULL`.
3. El último nodo de la cadena tiene `next == NULL`.
4. `list_pop` en lista vacía retorna `NULL`, no crashea.

Un bug que viola una invariante no se manifiesta en la función bugueada, sino varias llamadas después. Por eso Valgrind es esencial.

---

## Tema 4 — Manejo de errores POSIX: disciplina, no optimismo

### La filosofía del kernel: todo puede fallar

En C de sistemas, **cada llamada que adquiere un recurso puede fallar**:

| Tipo de función | Retorno de error | Código de error |
|-----------------|-----------------|-----------------|
| Syscalls (`open`, `read`, `write`) | `-1` | `errno` |
| Allocación (`malloc`, `calloc`) | `NULL` | `errno` (no siempre) |
| Stdio (`fopen`, `fgets`) | `NULL` o `EOF` | `errno` o `ferror` |
| Pthreads (`pthread_create`, etc.) | Código > 0 | El propio retorno (no `errno`) |

> [!CAUTION]
> Las funciones de pthreads son la excepción que todos olvidan: retornan el código de error directamente (ej: `EAGAIN`), **no usan `errno`**. Esto importará mucho en Bloque 5.

### `errno`: las 3 reglas de supervivencia

```c
int fd = open(path, O_RDONLY);
if (fd == -1) {
    int saved = errno;   // Regla 1: capturar INMEDIATAMENTE
    // Porque la siguiente llamada (incluso fprintf) puede modificar errno
    fprintf(stderr, "Error abriendo '%s': %s\n", path, strerror(saved));
    return -1;
}
```

| Regla | Detalle |
|-------|---------|
| 1. Solo leer si hubo error | `errno` puede contener basura de llamadas exitosas anteriores |
| 2. Capturar inmediatamente | Cualquier otra llamada (incluido `fprintf`) puede sobreescribir `errno` |
| 3. Es thread-local | Cada thread tiene su propio `errno` (desde POSIX.1-2001). Seguro en multihilo |

### Herramientas para reportar errores

```c
// perror: imprime "prefijo: mensaje de errno"
perror("open");
// Salida: "open: No such file or directory"

// strerror: retorna el string del error (no thread-safe en algunas plataformas)
fprintf(stderr, "Falló: %s\n", strerror(errno));

// strerror_r: versión thread-safe (hay dos variantes, cuidado)
char errbuf[256];
strerror_r(errno, errbuf, sizeof(errbuf));  // GNU vs POSIX difieren
```

### El patrón `goto cleanup`: el try/catch de C

C no tiene excepciones. El patrón estándar del kernel Linux y de prácticamente todo C de sistemas profesional es:

```c
int process_file(const char *path) {
    int rc = -1;           // Asumir fallo
    FILE *f = NULL;
    char *buf = NULL;
    int *results = NULL;

    buf = malloc(BUF_SIZE);
    if (!buf) { perror("malloc"); goto cleanup; }

    f = fopen(path, "r");
    if (!f) { perror(path); goto cleanup; }

    results = calloc(MAX_RESULTS, sizeof(int));
    if (!results) { perror("calloc"); goto cleanup; }

    // === Lógica principal ===
    while (fgets(buf, BUF_SIZE, f)) {
        // ... procesar ...
    }
    if (ferror(f)) { perror("fread"); goto cleanup; }

    rc = 0;  // Si llegamos aquí, todo fue bien

cleanup:
    free(results);    // free(NULL) es seguro
    if (f) fclose(f); // fclose(NULL) NO es seguro — verificar
    free(buf);
    return rc;
}
```

**¿Por qué `goto` y no anidar `if`?** Compara:

```c
// Sin goto: anidamiento creciente, limpieza duplicada
buf = malloc(1024);
if (buf) {
    f = fopen(path, "r");
    if (f) {
        results = calloc(100, sizeof(int));
        if (results) {
            // trabajo
            free(results);
        }
        fclose(f);
    }
    free(buf);
}
```

Con 5 recursos, tendrías 5 niveles de anidamiento. Con `goto cleanup`, siempre es plano.

### Macros de chequeo

Para no repetir el patrón `if (...) { ... goto cleanup; }` en cada línea:

```c
// Para syscalls que retornan -1 en error
#define CHECK_SYS(call) \
    do { \
        if ((call) == -1) { \
            fprintf(stderr, "[%s:%d] %s: %s\n", \
                    __FILE__, __LINE__, #call, strerror(errno)); \
            goto cleanup; \
        } \
    } while (0)

// Para funciones que retornan NULL en error
#define CHECK_PTR(ptr) \
    do { \
        if (!(ptr)) { \
            fprintf(stderr, "[%s:%d] %s es NULL: %s\n", \
                    __FILE__, __LINE__, #ptr, strerror(errno)); \
            goto cleanup; \
        } \
    } while (0)
```

Uso:

```c
int fd;
char *buf;

CHECK_SYS(fd = open(path, O_RDONLY));
CHECK_PTR(buf = malloc(4096));
CHECK_SYS(read(fd, buf, 4096));
// ...
```

> [!NOTE]
> **`do { ... } while (0)` no es decorativo.** Sin él, `if (cond) CHECK_SYS(x); else ...` se rompe porque el macro se expande en múltiples statements. El `do/while(0)` lo convierte en un statement único.

### Errores que requieren tratamiento especial

| `errno` | Significado | Qué hacer |
|---------|-------------|-----------|
| `EINTR` | La syscall fue interrumpida por una señal | Reintentar la llamada (importa mucho en Bloque 3) |
| `EAGAIN` / `EWOULDBLOCK` | Recurso no disponible temporalmente | Reintentar o usar I/O no-bloqueante |
| `EACCES` / `EPERM` | Sin permisos | Reportar al usuario con ruta y contexto |
| `ENOENT` | Archivo/directorio no existe | Reportar la ruta exacta que no se encontró |
| `ENOSPC` | Sin espacio en disco | Reportar y considerar limpieza |
| `ENOMEM` | Sin memoria | Generalmente irrecuperable; limpieza y salida |

---

## Tema 5 — Aritmética de punteros y microbenchmarks

### Operaciones de memoria a nivel de byte

Para escribir `memcpy`/`memmove`/`memset` propios, trabaja con `unsigned char *`:

```c
unsigned char *d = (unsigned char *)dest;
const unsigned char *s = (const unsigned char *)src;
```

¿Por qué `unsigned char` y no `char`? Porque `char` puede ser signed (depende de la plataforma), y un byte con valor 0xFF se interpretaría como -1 con signed char. `unsigned char` siempre va de 0 a 255.

### `my_memcpy`: la versión didáctica

```c
void *my_memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    for (size_t i = 0; i < n; i++)
        d[i] = s[i];
    return dest;
}
```

> [!WARNING]
> **`memcpy` tiene una restricción que `memmove` no tiene:** si las regiones de origen y destino se solapan, el comportamiento es indefinido. Esto no es un "puede causar problemas" — es literalmente UB según el estándar.

### `my_memmove`: manejando solapamiento

La clave es detectar la dirección del solapamiento:

```c
void *my_memmove(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;

    if (d < s) {
        // Copiar adelante: d está antes que s, no se pisan
        for (size_t i = 0; i < n; i++)
            d[i] = s[i];
    } else if (d > s) {
        // Copiar atrás: d está después que s, copiar desde el final
        for (size_t i = n; i > 0; i--)
            d[i-1] = s[i-1];
    }
    // Si d == s, no hacer nada
    return dest;
}
```

**Visualización del problema:**

```
src:  [A B C D E F]
dest:     [A B C D E F]   ← dest empieza 2 posiciones después

Copiando adelante: src[0]→dest[0] sobreescribe src[2] antes de leerlo.
Copiando atrás: empezamos por el final, sin pisar nada.
```

### `my_memset`

```c
void *my_memset(void *dest, int c, size_t n) {
    unsigned char *d = dest;
    for (size_t i = 0; i < n; i++)
        d[i] = (unsigned char)c;
    return dest;
}
```

### Benchmarking correcto: checklist anti-autoengaño

| Regla | Incorrecto | Correcto |
|-------|-----------|----------|
| Reloj adecuado | `time()` (resolución de 1 segundo) | `clock_gettime(CLOCK_MONOTONIC, &ts)` |
| Múltiples iteraciones | Medir una sola ejecución | Mínimo 100-1000 iteraciones, reportar mediana |
| Warm-up | Medir la primera iteración (caché frío) | Descartar las primeras N iteraciones |
| Evitar eliminación por el compilador | El compilador ve que el resultado no se usa → elimina el loop | Usar `volatile` o imprimir un checksum |
| Condiciones iguales | Debug (`-O0`) vs tú, Release (`-O2`) vs libc | Mismo `-O2` para ambos |
| Sistema estable | Correr con browser abierto y Docker compilando | Sistema en reposo, medir varias veces |

Plantilla de benchmark:

```c
#include <time.h>

static double time_diff_ns(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
}

void benchmark(void) {
    const int ITERS = 1000;
    const size_t SIZE = 1024 * 1024;  // 1 MB
    char *src = malloc(SIZE);
    char *dst = malloc(SIZE);
    memset(src, 'A', SIZE);

    // Warm-up
    for (int i = 0; i < 10; i++) my_memcpy(dst, src, SIZE);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (int i = 0; i < ITERS; i++) {
        my_memcpy(dst, src, SIZE);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);

    double ns_per_iter = time_diff_ns(t0, t1) / ITERS;
    double gb_per_sec = (SIZE / 1e9) / (ns_per_iter / 1e9);
    printf("my_memcpy: %.1f ns/iter, %.2f GB/s\n", ns_per_iter, gb_per_sec);

    free(src);
    free(dst);
}
```

### Por qué glibc gana: lo que tu bucle `for` no puede hacer

`memcpy` en glibc no es un bucle. Es código ensamblador mano-tuneado que:

1. **Detecta la CPU en runtime** (via IFUNC) y selecciona la ruta más rápida.
2. **Usa instrucciones SIMD** (SSE2/AVX2/AVX-512) que copian 16/32/64 bytes por instrucción.
3. **Maneja alineación** con un prólogo que alinea el destino a 16/32 bytes.
4. **Tiene rutas especiales** para copias pequeñas (<32 bytes) vs grandes (>256 KB, usa non-temporal stores para evitar contaminar caché).

Tu implementación existe para **entender cómo funciona la copia de memoria**, no para reemplazar a libc.

---

## Tema 6 — El proyecto integrador: `miniecho` y `minicat`

Todo lo anterior converge aquí. Estas no son herramientas de juguete; son clones funcionales de utilidades reales que deben pasar esta regla: **si reemplazas `echo`/`cat` por tu versión, nada debería romperse**.

### `miniecho`

Opciones a soportar:
- `-n`: No imprimir newline al final.
- `-e`: Interpretar secuencias de escape.

Secuencias de escape que `-e` debe manejar:

| Secuencia | Significado |
|-----------|-------------|
| `\\n` | Newline |
| `\\t` | Tab |
| `\\\\` | Backslash literal |
| `\\xHH` | Byte hexadecimal (ej: `\\x41` → `'A'`) |
| `\\c` | **Detener salida inmediatamente** (no imprime nada más, ni newline) |

El punto fino: `\\c` no solo suprime el newline final — corta la salida por completo, incluso si hay más argumentos después.

### `minicat`

Opciones a soportar:
- `-n`: Numerar todas las líneas.
- `-b`: Numerar solo líneas no vacías (override `-n`).
- `-s`: Comprimir múltiples líneas vacías consecutivas en una sola.
- `-` como argumento: Leer de stdin.

**Manejo de errores multi-archivo:**

```bash
./minicat archivo1.txt no_existe.txt archivo3.txt
```

El programa debe:
1. Imprimir `archivo1.txt` normalmente.
2. Imprimir error en `stderr` para `no_existe.txt`.
3. **Continuar** con `archivo3.txt` (no abortar).
4. Al final, retornar `EXIT_FAILURE` si hubo **al menos un** error.

### Filosofía Unix: las 4 leyes

Todos los programas que construyas en este curso (y en tu carrera) deben cumplir:

1. **Hacer una cosa bien.** `echo` solo imprime. `cat` solo concatena.
2. **Salida limpia para pipes.** `./mi_tool | sort | uniq` debe funcionar.
3. **Errores ruidosos.** Si algo falla, el usuario debe enterarse. Silencio = engaño.
4. **Contrato estable.** Las mismas opciones producen el mismo resultado siempre.

---

## Checklist de salida del Bloque 01

Antes de avanzar al Bloque 02, verifica:

- [ ] Puedes escribir un parser CLI completo con `getopt_long` (opciones cortas y largas)
- [ ] Tus wrappers de string nunca desbordan buffers y siempre null-terminan
- [ ] Tu lista enlazada genérica tiene 0 leaks verificados con Valgrind
- [ ] Usas `goto cleanup` con macros `CHECK_SYS`/`CHECK_PTR` en toda función que adquiere recursos
- [ ] Puedes medir la diferencia de rendimiento entre tu `memcpy` y el de libc con un benchmark válido
- [ ] `miniecho` y `minicat` producen output idéntico a los originales en los tests

> Si cualquiera de estos puntos falla, en los bloques siguientes (procesos, señales, memoria compartida) los bugs serán difíciles de rastrear porque **no sabrás si el problema es del nuevo concepto o de los fundamentos**.

---

## Referencias

| Recurso | Comando |
|---------|---------|
| Parseo de opciones | `man 3 getopt`, `man 3 getopt_long` |
| Conversión numérica | `man 3 strtol`, `man 3 strtoul` |
| Strings y formato | `man 3 snprintf`, `man 3 vsnprintf` |
| Memoria | `man 3 malloc`, `man 3 free`, `man 3 realloc` |
| Errores | `man 3 errno`, `man 3 perror`, `man 3 strerror` |
| Copia de memoria | `man 3 memcpy`, `man 3 memmove`, `man 3 memset` |
| Benchmarking | `man 3 clock_gettime` |
