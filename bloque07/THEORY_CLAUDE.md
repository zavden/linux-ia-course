# THEORY_CLAUDE.md — Bloque 07: Debugging, Build Systems y Calidad de Código en C

> Los bloques anteriores te enseñaron a construir software de sistemas. Este bloque te enseña a **mantenerlo, depurarlo y hacerlo profesional**. La diferencia entre un proyecto hobby y uno de producción no está en la lógica — está en los builds reproducibles, los errores trazables, los tests que detectan regresiones, y la capacidad de diagnosticar un crash a las 3 AM con un core dump.

---

## Mapa del Bloque

```
Tema 1: Debug vs Release     →  Flags, assert, NDEBUG
Tema 2: GDB                  →  Breakpoints, backtrace, inspección de estado
Tema 3: Core dumps           →  Análisis post-mortem sin reproducir el bug
Tema 4: Sanitizers           →  ASan, UBSan, TSan — detección automatizada
Tema 5: Valgrind             →  Auditoría profunda de memoria
Tema 6: Error reporting      →  Mensajes útiles con contexto
Tema 7: Build system         →  Makefiles modulares, perfiles, dependencias
Tema 8: Preprocesador        →  Feature flags, macros de log, plataformas
Tema 9: Librerías            →  Estáticas (.a) vs compartidas (.so)
Tema 10: Instrumentación     →  Medir antes de optimizar
```

---

## Tema 1 — Debug vs Release: dos mundos

### Las dos configuraciones

| Aspecto | Debug | Release |
|---------|-------|---------|
| **Objetivo** | Máxima observabilidad | Máximo rendimiento |
| **Optimización** | `-O0` (nada) o `-Og` (debug-friendly) | `-O2` o `-O3` |
| **Símbolos** | `-g` (tabla de funciones y líneas) | Opcional (`-g` no afecta runtime) |
| **Sanitizers** | `-fsanitize=address,undefined` | Nunca |
| **Asserts** | Activos | `-DNDEBUG` los desactiva |
| **Binario** | Grande, lento, informativo | Pequeño, rápido, opaco |

```makefile
# En tu Makefile:
ifdef DEBUG
  CFLAGS += -g -O0 -fsanitize=address,undefined
  LDFLAGS += -fsanitize=address,undefined
else
  CFLAGS += -O2 -DNDEBUG
endif
```

### `assert`: documentar invariantes, no validar input

```c
#include <assert.h>

void process_buffer(char *buf, size_t len) {
    assert(buf != NULL);     // invariante interna: el caller garantiza esto
    assert(len > 0);         // si esto falla, es un bug del caller, no del usuario
    // ...
}
```

`assert(expr)` aborta el programa con un mensaje si `expr` es falso:

```
prog: main.c:42: process_buffer: Assertion `buf != NULL' failed.
Aborted (core dumped)
```

> [!CAUTION]
> **No pongas lógica con efectos secundarios dentro de `assert`.** Con `-DNDEBUG`, la expresión se elimina completamente:
> ```c
> // ❌ BUG: en release, el read no se ejecuta
> assert(read(fd, buf, n) > 0);
>
> // ✅ CORRECTO:
> ssize_t r = read(fd, buf, n);
> assert(r > 0);
> ```

---

## Tema 2 — GDB: depuración interactiva

### Flujo de trabajo

```bash
# 1. Compilar con símbolos
gcc -g -O0 -o programa src/main.c src/utils.c

# 2. Iniciar GDB
gdb ./programa

# 3. Dentro de GDB:
(gdb) break main              # breakpoint en función
(gdb) break src/utils.c:42    # breakpoint en archivo:línea
(gdb) run arg1 arg2           # ejecutar con argumentos
```

### Comandos esenciales

| Comando | Atajo | Qué hace |
|---------|-------|----------|
| `run [args]` | `r` | Ejecutar el programa |
| `break <loc>` | `b` | Poner breakpoint |
| `continue` | `c` | Continuar hasta siguiente breakpoint |
| `next` | `n` | Ejecutar siguiente línea (sin entrar en funciones) |
| `step` | `s` | Ejecutar siguiente línea (entrando en funciones) |
| `print <expr>` | `p` | Evaluar una expresión C |
| `backtrace` | `bt` | Mostrar stack de llamadas |
| `frame <n>` | `f` | Cambiar al frame n del stack |
| `info locals` | | Mostrar variables locales del frame actual |
| `info args` | | Mostrar argumentos de la función actual |
| `watch <var>` | | Detener cuando la variable cambie de valor |
| `list` | `l` | Mostrar código fuente alrededor del punto actual |

### Inspeccionar estructuras de datos

```c
(gdb) print *node              // desreferenciar puntero
(gdb) print node->next->data   // seguir cadena de punteros
(gdb) print arr[0]@10          // ver 10 elementos de un array
(gdb) x/16xb ptr               // ver 16 bytes en hexadecimal desde ptr
(gdb) print (char *)buf        // castear tipo para mejor display
```

### Debugging de crashes

```bash
(gdb) run
# ... el programa crashea con SIGSEGV ...
Program received signal SIGSEGV, Segmentation fault.
0x0000555555555189 in process_line (line=0x0) at parser.c:27
27          int len = strlen(line);

(gdb) bt
#0  0x000055... in process_line (line=0x0) at parser.c:27
#1  0x000055... in main_loop () at main.c:85
#2  0x000055... in main (argc=1, argv=0x7fff...) at main.c:12

(gdb) frame 1                  # ir al frame del caller
(gdb) print line               # ver qué valor tenía line
$1 = 0x0                       # NULL: aquí está el bug
```

### GDB con múltiples hilos (repaso del Bloque 5)

```bash
(gdb) info threads             # listar todos los hilos
(gdb) thread 3                 # cambiar al hilo 3
(gdb) thread apply all bt      # backtrace de TODOS los hilos
```

---

## Tema 3 — Core dumps: la caja negra del crash

### Qué es un core dump

Cuando un proceso muere por señal fatal (`SIGSEGV`, `SIGABRT`, `SIGFPE`), el kernel puede escribir un archivo con **todo el estado del proceso**: memoria, registros, stack. Esto permite **analizar el crash sin necesidad de reproducirlo**.

### Habilitar core dumps

```bash
# Verificar límite actual:
ulimit -c
# Si dice 0, los core dumps están deshabilitados

# Habilitar:
ulimit -c unlimited

# Verificar dónde se guardan:
cat /proc/sys/kernel/core_pattern
# Típico: core, core.%p, o un path de systemd-coredump
```

En Docker:

```dockerfile
# Dentro del Dockerfile o al ejecutar:
RUN ulimit -c unlimited
```

```bash
docker run --ulimit core=-1 ...
```

### Análisis post-mortem con GDB

```bash
# El programa crasheó y generó "core" o "core.1234"
gdb ./programa core

(gdb) bt                       # ¿dónde crasheó?
#0  0x000055... in parse_header (buf=0x7f..., len=0) at parser.c:55
#1  0x000055... in handle_request (conn=0x55...) at server.c:112

(gdb) frame 0
(gdb) info locals              # ¿cuál era el estado?
buf = 0x7f4a2c001000
len = 0                        # len era 0 → acceso a buf[0] con len=0 es el bug

(gdb) print *conn              # inspeccionar la conexión
```

> [!TIP]
> **Core dumps son invaluables para bugs en producción.** Un bug que tarda horas en reproducirse se analiza en minutos con un core dump. Configura tus servidores para generarlos siempre.

---

## Tema 4 — Sanitizers: detección automática de bugs

### AddressSanitizer (ASan)

Detecta errores de **límites de memoria** en tiempo de ejecución:

```bash
gcc -g -O0 -fsanitize=address -o programa src/main.c
./programa
```

| Qué detecta | Ejemplo |
|-------------|---------|
| Heap buffer overflow | `malloc(10); p[10] = 'x';` |
| Stack buffer overflow | `char buf[10]; buf[10] = 'x';` |
| Use-after-free | `free(p); *p = 42;` |
| Double free | `free(p); free(p);` |
| Memory leaks | `malloc` sin `free` (con `ASAN_OPTIONS=detect_leaks=1`) |

Salida típica:

```
=================================================================
==1234==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x60200000001a
READ of size 1 at 0x60200000001a thread T0
    #0 0x555... in process_line src/parser.c:27
    #1 0x555... in main src/main.c:12

0x60200000001a is located 0 bytes after 10-byte region [0x602000000010,0x60200000001a)
allocated by thread T0 here:
    #0 0x7f4... in malloc (/usr/lib/libasan.so.6)
    #1 0x555... in read_input src/io.c:45
```

### UndefinedBehaviorSanitizer (UBSan)

Detecta **comportamiento indefinido** que el compilador no está obligado a manejar:

```bash
gcc -g -O0 -fsanitize=undefined -o programa src/main.c
```

| Qué detecta | Ejemplo |
|-------------|---------|
| Signed integer overflow | `INT_MAX + 1` |
| Division by zero | `x / 0` |
| Null pointer dereference | `int *p = NULL; *p;` |
| Shift past bit width | `1 << 33` (en int de 32 bits) |
| Misaligned access | Leer `int` desde dirección no alineada |

### Combinaciones y compatibilidades

| Combinación | ¿Funciona? | Notas |
|-------------|-----------|-------|
| ASan + UBSan | ✅ | La combinación recomendada para desarrollo |
| ASan + Valgrind | ❌ | Ambos interceptan malloc, se interfieren |
| TSan + ASan | ❌ | Mutuamente excluyentes |
| TSan solo | ✅ | Para bugs de concurrencia (Bloque 5) |

```bash
# Combinación estándar para desarrollo:
gcc -g -O0 -fsanitize=address,undefined -fno-omit-frame-pointer programa.c

# Para concurrencia:
gcc -g -O1 -fsanitize=thread programa.c -pthread
```

> [!NOTE]
> **`-fno-omit-frame-pointer`** mejora los stacktraces de ASan. Sin él, algunas funciones pueden aparecer como `??` en el backtrace.

---

## Tema 5 — Valgrind: auditoría profunda

### Cuándo usar Valgrind en vez de ASan

| Necesidad | ASan | Valgrind |
|-----------|------|----------|
| Velocidad | ~2x slowdown | ~20x slowdown |
| Detección de leaks | Básica | Muy precisa (categorizada) |
| Reads no inicializados | No | Sí (`--track-origins=yes`) |
| Requiere recompilación | Sí (con `-fsanitize`) | No (binario normal con `-g`) |

```bash
# Compilar SIN sanitizers, SOLO con -g:
gcc -g -O0 -o programa src/main.c

# Ejecutar bajo Valgrind:
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./programa
```

### Leer la salida de Valgrind

```
==1234== HEAP SUMMARY:
==1234==     in use at exit: 128 bytes in 1 blocks
==1234==   total heap usage: 5 allocs, 4 frees, 2,176 bytes allocated
==1234==
==1234== 128 bytes in 1 blocks are definitely lost in loss record 1 of 1
==1234==    at 0x4C2FB0F: malloc (vg_replace_malloc.c:381)
==1234==    by 0x10916A: parse_config (config.c:42)      ← aquí se allocó
==1234==    by 0x1091B2: main (main.c:15)
```

| Categoría | Significado | Gravedad |
|-----------|-------------|----------|
| **Definitely lost** | Leak seguro: no quedan punteros al bloque | 🔴 Arreglar siempre |
| **Indirectly lost** | Apuntado solo desde un bloque que está "definitely lost" | 🟡 Se arregla al arreglar el padre |
| **Possibly lost** | Hay un puntero al medio del bloque (no al inicio) | 🟡 Investigar |
| **Still reachable** | Hay punteros al bloque pero no se liberó | 🟢 A menudo aceptable (globals) |

---

## Tema 6 — Error reporting: mensajes que ayudan

### El formato profesional

```
programa: operación recurso: razón del sistema
```

```c
// ❌ INÚTIL para diagnóstico:
printf("error\n");
fprintf(stderr, "falló\n");

// ✅ ÚTIL:
fprintf(stderr, "%s: open '%s': %s\n", argv[0], path, strerror(errno));
// Salida: "miniserver: open '/etc/config.env': Permission denied"
```

### Macro de logging con contexto

```c
#define LOG_ERROR(fmt, ...) \
    fprintf(stderr, "[ERROR] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_WARN(fmt, ...) \
    fprintf(stderr, "[WARN]  %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define LOG_INFO(fmt, ...) \
    fprintf(stderr, "[INFO]  " fmt "\n", ##__VA_ARGS__)

#ifdef DEBUG
#define LOG_DEBUG(fmt, ...) \
    fprintf(stderr, "[DEBUG] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...) ((void)0)
#endif
```

Uso:

```c
LOG_INFO("Servidor iniciando en puerto %d", port);
LOG_DEBUG("Aceptando conexión de %s", ip_str);
LOG_ERROR("bind falló en puerto %d: %s", port, strerror(errno));
```

> [!TIP]
> **`##__VA_ARGS__`** es una extensión de GCC/Clang que elimina la coma si no hay argumentos variadicos. Sin él, `LOG_INFO("hola")` no compilaría porque sobraría una coma.

---

## Tema 7 — Build system: Makefiles modulares

### Proyecto multiarchivo con perfiles

```makefile
CC       := gcc
CFLAGS   := -Wall -Wextra -Werror -pedantic -std=c17
LDFLAGS  :=
LDLIBS   :=

SRC_DIR  := src
BUILD_DIR := build
SRC      := $(wildcard $(SRC_DIR)/*.c)
OBJ      := $(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEP      := $(OBJ:.o=.d)
TARGET   := $(BUILD_DIR)/programa

# === Perfiles ===
ifdef DEBUG
  CFLAGS  += -g -O0 -fsanitize=address,undefined
  LDFLAGS += -fsanitize=address,undefined
else ifdef PROFILE
  CFLAGS  += -g -O2 -pg
  LDFLAGS += -pg
else
  CFLAGS  += -O2 -DNDEBUG
endif

# === Targets ===
.PHONY: all clean test debug profile

all: $(TARGET)

debug:
	$(MAKE) DEBUG=1 all

profile:
	$(MAKE) PROFILE=1 all

test: $(TARGET)
	@echo "=== Running tests ==="
	@bash tests/run_tests.sh

clean:
	rm -rf $(BUILD_DIR)

# === Reglas ===
$(TARGET): $(OBJ) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

-include $(DEP)
```

### Librerías internas

Para proyectos con múltiples componentes:

```makefile
# Compilar librería estática interna
LIB_SRC := $(wildcard lib/*.c)
LIB_OBJ := $(LIB_SRC:lib/%.c=$(BUILD_DIR)/lib/%.o)

$(BUILD_DIR)/libutil.a: $(LIB_OBJ)
	ar rcs $@ $^

$(BUILD_DIR)/lib/%.o: lib/%.c | $(BUILD_DIR)/lib
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/lib:
	mkdir -p $@

# El binario principal depende de la librería
$(TARGET): $(OBJ) $(BUILD_DIR)/libutil.a
	$(CC) $(LDFLAGS) $(OBJ) -L$(BUILD_DIR) -lutil $(LDLIBS) -o $@
```

---

## Tema 8 — Preprocesador: feature flags y macros

### Feature flags con `-D`

```bash
gcc -DUSE_EPOLL -DLOG_LEVEL=2 src/main.c
```

```c
#if defined(USE_EPOLL)
    #include "reactor_epoll.h"
#elif defined(USE_KQUEUE)
    #include "reactor_kqueue.h"
#else
    #include "reactor_poll.h"   // fallback portable
#endif
```

### Include guards (obligatorio en todo `.h`)

```c
// utils.h
#ifndef UTILS_H
#define UTILS_H

// declaraciones...
void util_log(const char *msg);

#endif // UTILS_H
```

Sin include guards, incluir el mismo header dos veces (directa o indirectamente) causa errores de "múltiple definición".

### Macros parametrizadas: útiles pero peligrosas

```c
// ✅ Macro útil con do/while(0):
#define SAFE_FREE(ptr) do { free(ptr); (ptr) = NULL; } while (0)

// ❌ Macro peligrosa — efecto secundario múltiple:
#define MAX(a, b) ((a) > (b) ? (a) : (b))
// MAX(i++, j++) → i o j se incrementan DOS VECES
```

> [!WARNING]
> **`MAX(a, b)` como macro es un antipatrón clásico.** Si `a` o `b` tienen efectos secundarios (como `i++`), se evalúan dos veces. En GNU C, usa `__typeof__` y un bloque de statement expression. En general, para este curso, prefiere funciones inline:
> ```c
> static inline int max_int(int a, int b) { return a > b ? a : b; }
> ```

---

## Tema 9 — Librerías estáticas vs compartidas

### Estática (`.a`)

```bash
# Compilar objetos
gcc -c src/utils.c -o build/utils.o
gcc -c src/parse.c -o build/parse.o

# Crear librería estática
ar rcs build/libutil.a build/utils.o build/parse.o

# Usar
gcc src/main.c -Lbuild -lutil -o build/programa
```

El linker copia las funciones usadas directamente al binario final. El resultado es autosuficiente.

### Compartida (`.so`)

```bash
# Compilar con position-independent code
gcc -fPIC -c src/utils.c -o build/utils.o
gcc -fPIC -c src/parse.c -o build/parse.o

# Crear librería compartida
gcc -shared -o build/libutil.so build/utils.o build/parse.o

# Usar
gcc src/main.c -Lbuild -lutil -o build/programa

# Ejecutar (necesita encontrar la .so):
LD_LIBRARY_PATH=./build ./build/programa
```

### Comparación

| Aspecto | Estática `.a` | Compartida `.so` |
|---------|---------------|------------------|
| Tamaño del binario | Mayor | Menor |
| Dependencias en runtime | Ninguna | Necesita la `.so` instalada |
| Actualización de la lib | Recompilar todo | Solo reemplazar `.so` |
| Complejidad de deploy | Mínima | Gestionar paths (`LD_LIBRARY_PATH`, `rpath`) |
| Para este curso | ✓ Recomendada | Conocer, no exigida |

---

## Tema 10 — Instrumentación: medir antes de optimizar

### La regla de oro

> "No optimices sin medir. Tu intuición sobre dónde está el cuello de botella es casi siempre incorrecta."

### Macros de timing

```c
#include <time.h>

#define TIMER_START(name) \
    struct timespec _timer_##name##_start; \
    clock_gettime(CLOCK_MONOTONIC, &_timer_##name##_start)

#define TIMER_END(name) do { \
    struct timespec _timer_end; \
    clock_gettime(CLOCK_MONOTONIC, &_timer_end); \
    double _elapsed = (_timer_end.tv_sec - _timer_##name##_start.tv_sec) * 1000.0 \
                    + (_timer_end.tv_nsec - _timer_##name##_start.tv_nsec) / 1e6; \
    fprintf(stderr, "[TIMER] %s: %.3f ms\n", #name, _elapsed); \
} while (0)
```

Uso:

```c
TIMER_START(parse);
parse_config(path);
TIMER_END(parse);
// Output: [TIMER] parse: 0.342 ms

TIMER_START(process);
process_all_records(data, n);
TIMER_END(process);
// Output: [TIMER] process: 145.721 ms  ← este es el hotspot
```

### `strace -c`: contar syscalls

```bash
strace -c ./programa
```

```
% time     calls  syscall
------ --------- ---------------
 45.32      4096  write
 31.21      4096  read
 12.11         1  open
  ...
```

Si ves 4096 `read` calls, tu buffer probablemente es demasiado pequeño.

---

## Checklist de salida del Bloque 07

- [ ] Compilar el mismo código en debug y release con un solo `make DEBUG=1` vs `make`
- [ ] Reproducir un crash en GDB, obtener backtrace y diagnosticar la causa
- [ ] Generar y analizar un core dump post-mortem
- [ ] Pasar ASan + UBSan sin errores
- [ ] Pasar Valgrind con 0 leaks "definitely lost"
- [ ] Todos los mensajes de error incluyen: programa, operación, recurso, y `strerror(errno)`
- [ ] El Makefile tiene targets `all`, `clean`, `test`, `debug`, `run`
- [ ] Macros de log con niveles (ERROR, WARN, INFO, DEBUG) controlables por compilación
- [ ] Crear y usar una librería estática interna
- [ ] Medir al menos un hotspot con `clock_gettime` y verificar optimización

---

## Referencias

| Recurso | Comando/Link |
|---------|-------------|
| GDB | `man gdb`, [GDB cheatsheet](https://darkdust.net/files/GDB%20Cheat%20Sheet.pdf) |
| Core dumps | `man 5 core`, `ulimit -c` |
| ASan | [ASan docs](https://clang.llvm.org/docs/AddressSanitizer.html) |
| UBSan | [UBSan docs](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html) |
| Valgrind | `man 1 valgrind`, [Valgrind quick start](https://valgrind.org/docs/manual/quick-start.html) |
| Make | `man make`, [GNU Make manual](https://www.gnu.org/software/make/manual/) |
| strace | `man 1 strace` |
| Preprocesador | `man cpp`, `gcc -E` para inspeccionar |
