# THEORY_CLAUDE.md — Bloque 00: Entorno, Toolchain y Docker para C en Linux

> Este bloque construye los cimientos del curso entero. Todo lo que hagas aquí — cómo compilas, cómo detectas errores de memoria, cómo reproduces un entorno — determina si los bloques de procesos, sockets y kernel serán un aprendizaje productivo o una lucha constante contra bugs invisibles.

---

## Mapa del Bloque

```
Tema 1: Toolchain GCC         →  Entender qué hace el compilador, paso a paso
Tema 2: GNU Make               →  Automatizar builds con dependencias reales
Tema 3: Modelo de memoria      →  Heap, stack, y las 4 categorías de bug mortal
Tema 4: Docker como laboratorio →  Entornos reproducibles Fedora + Debian
Tema 5: Configuración robusta  →  Variables de entorno, fallback, validación
```

---

## Tema 1 — Toolchain C: Lo que GCC hace cuando tú no miras

### El pipeline completo

Cuando ejecutas `gcc main.c -o main`, pareciera que ocurre una sola cosa. En realidad son **cuatro programas distintos** ejecutándose en secuencia:

```
main.c ──▶ Preprocesador ──▶ Compilador ──▶ Ensamblador ──▶ Linker ──▶ main
           (-E → .i)         (-S → .s)      (-c → .o)       (→ ELF)
```

Puedes invocar cada fase por separado para diagnosticar problemas:

```bash
# 1. Preprocesado: expande #include, macros, #ifdef
gcc -E main.c -o main.i

# 2. Compilación: C → Ensamblador (arquitectura específica)
gcc -S main.i -o main.s

# 3. Ensamblado: Ensamblador → Código máquina (archivo objeto)
gcc -c main.s -o main.o

# 4. Enlazado: resuelve símbolos entre objetos y produce el binario
gcc main.o -o main
```

> [!TIP]
> **¿Cuándo necesitas esto?** Cuando ves errores que no entiendes. Un "undefined reference" es un error de **enlazado** (fase 4), no de compilación. Un macro que no se expande como esperas se diagnostica inspeccionando el `.i` (fase 1). Saber en qué fase ocurre el error te lleva a la solución correcta.

### Flags obligatorias del curso

Cada línea de C en este curso se compila con:

```bash
gcc -Wall -Wextra -Werror -pedantic -std=c17
```

| Flag | Qué hace | Por qué importa |
|------|----------|------------------|
| `-Wall` | Activa la mayoría de warnings comunes | Detecta variables sin usar, comparaciones sospechosas, etc. |
| `-Wextra` | Warnings adicionales que `-Wall` no cubre | Parámetros no usados, conversiones implícitas dudosas |
| `-Werror` | **Todo warning se convierte en error** | Impide compilar código con deuda técnica |
| `-pedantic` | Rechaza extensiones de GCC no estándar | Tu código será C real, portable entre compiladores |
| `-std=c17` | Fija el dialecto a C17 | Sin sorpresas por el default cambiante del compilador |

### Perfiles: Debug vs Release

Son dos modos mutuamente excluyentes. Nunca mezcles benchmark con sanitizers.

```bash
# Debug: máxima información, cero optimización, detección de errores en runtime
gcc -Wall -Wextra -Werror -pedantic -std=c17 -g -O0 -fsanitize=address,undefined

# Release: máxima velocidad, sin instrumentación
gcc -Wall -Wextra -Werror -pedantic -std=c17 -O2
```

- **`-g`**: Incluye tabla de símbolos (nombres de funciones, líneas) para GDB.
- **`-O0`**: Desactiva optimizaciones. El compilador no reordena ni elimina código, así que el debugger muestra exactamente lo que escribiste.
- **`-fsanitize=address`** (ASan): Inyecta comprobaciones de límites en cada acceso a memoria.
- **`-fsanitize=undefined`** (UBSan): Detecta comportamiento indefinido (overflow de enteros con signo, divisiones por cero, etc.).

> [!WARNING]
> **`-O2` + `-fsanitize=address` es una combinación engañosa.** El optimizador puede eliminar código que ASan esperaba instrumentar. Resultado: bugs que existen pero que el sanitizer no reporta. Usa siempre `-O0` con sanitizers.

### Linking: estático vs dinámico

| Tipo | Extensión | Ventaja | Desventaja |
|------|-----------|---------|------------|
| Estático | `.a` | Binario autosuficiente, funciona sin dependencias instaladas | Más grande, no recibe parches de la librería del sistema |
| Dinámico | `.so` | Binario pequeño, se actualiza con el sistema | Falla si la `.so` no está instalada o cambió de versión |

Crear y usar una librería estática:

```bash
# Compilar objetos
gcc -c src/utils.c -o build/utils.o
gcc -c src/parse.c -o build/parse.o

# Empaquetar en archivo estático
ar rcs build/libmyutil.a build/utils.o build/parse.o

# Usar: -L indica directorio, -l indica nombre (sin prefijo lib ni extensión)
gcc src/main.c -Lbuild -lmyutil -o build/app
```

### Los 4 errores de toolchain que verás repetidamente

| Error | Fase | Causa | Solución |
|-------|------|-------|----------|
| `undefined reference to 'foo'` | Enlazado | Falta el `.o` o la librería que define `foo` | Añadir el objeto/librería al comando de link |
| `multiple definition of 'bar'` | Enlazado | `bar` definida en dos `.c` distintos | Declarar en `.h`, definir en un solo `.c` |
| `#include file not found` | Preprocesado | Ruta de header incorrecta o falta `-I` | Verificar path, añadir `-Iinclude/` |
| `implicit declaration of function` | Compilación | Falta el `#include` del header que declara la función | Incluir el header correcto |

---

## Tema 2 — GNU Make: No es un script, es un grafo de dependencias

### El error conceptual más común

Muchos tratan el `Makefile` como un script lineal tipo bash. No lo es. Make construye un **grafo dirigido acíclico** de dependencias basado en timestamps de archivos. Solo reconstruye lo que cambió.

```
Si timestamp(src/math.c) > timestamp(build/math.o):
    recompilar build/math.o
    re-enlazar build/app
Si no:
    no hacer nada
```

### Makefile profesional mínimo

```makefile
CC      := gcc
CFLAGS  := -Wall -Wextra -Werror -pedantic -std=c17
LDFLAGS :=

SRC     := $(wildcard src/*.c)
OBJ     := $(SRC:src/%.c=build/%.o)
DEP     := $(OBJ:.o=.d)       # Archivos de dependencia automática
TARGET  := build/app

# === Perfil debug/release ===
ifdef DEBUG
  CFLAGS  += -g -O0 -fsanitize=address,undefined
  LDFLAGS += -fsanitize=address,undefined
else
  CFLAGS  += -O2
endif

# === Targets ===
.PHONY: all run test clean debug

all: $(TARGET)

debug:
	$(MAKE) DEBUG=1 all

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	@bash tests/run_tests.sh

clean:
	rm -rf build

# === Reglas de compilación ===
$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

# -MMD genera .d con dependencias de headers
# -MP añade targets vacíos para headers borrados (evita errores de make)
build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

build:
	mkdir -p build

# Incluir dependencias generadas (el '-' silencia error si no existen aún)
-include $(DEP)
```

### Conceptos que debes dominar

| Concepto | Sintaxis | Significado |
|----------|----------|-------------|
| Asignación simple | `CC := gcc` | Evalúa inmediatamente |
| Asignación lazy | `CC = gcc` | Evalúa cada vez que se usa (puede ser costoso) |
| Condicional | `CC ?= gcc` | Solo asigna si la variable no tiene valor |
| Append | `CFLAGS += -g` | Añade al valor existente |
| Target automático | `$@` | El nombre del target que se está construyendo |
| Primera dependencia | `$<` | La primera dependencia de la regla |
| Todas las dependencias | `$^` | Todas las dependencias, sin repetir |
| `.PHONY` | `.PHONY: clean` | Indica que `clean` no es un archivo real |
| Order-only | `| build` | Dependencia que solo verifica existencia, no timestamp |

> [!CAUTION]
> **Tabs, no espacios.** Las recetas (las líneas que ejecutan comandos bajo un target) **deben** usar un carácter TAB real. Espacios causan `*** missing separator. Stop.` — un error que confunde a todos la primera vez.

### Dependencias automáticas de headers: por qué importan

Sin `-MMD -MP`, si cambias un `.h`, Make no lo sabe y **no recompila** los `.c` que lo incluyen. Tu programa compila con headers viejos. El resultado es un bug fantasma donde el código fuente dice una cosa y el binario hace otra.

---

## Tema 3 — Memoria en C: Modelo mental y diagnóstico

### El mapa de memoria de un proceso Linux

Cuando el kernel carga tu binario ELF, organiza la memoria del proceso así:

```
Dirección alta ┌──────────────────────┐
               │  Stack               │ ← Variables locales, frames de función
               │  ↓ crece hacia abajo │    Tamaño limitado (típicamente 8 MB)
               ├──────────────────────┤
               │                      │
               │  (espacio libre)     │
               │                      │
               ├──────────────────────┤
               │  ↑ crece hacia arriba│
               │  Heap                │ ← malloc/calloc/realloc
               ├──────────────────────┤
               │  BSS                 │ ← Globales no inicializadas (zeroed)
               ├──────────────────────┤
               │  Data                │ ← Globales inicializadas
               ├──────────────────────┤
               │  Read-only data      │ ← Literales de string, constantes
               ├──────────────────────┤
Dirección baja │  Text (código)       │ ← Las instrucciones de tu programa
               └──────────────────────┘
```

Cada sección tiene reglas distintas. Escribir en `Text` o `Read-only data` causa `SIGSEGV`. El `Stack` tiene tamaño fijo. El `Heap` crece bajo demanda pero requiere gestión manual.

### Las 4 categorías de bug de memoria

Todo error de memoria en C cae en una de estas categorías:

#### 1. Memory leak — Pides y nunca devuelves

```c
void process(void) {
    char *buf = malloc(4096);
    if (!buf) return;
    // ... usar buf ...
    return;  // BUG: nunca se llama free(buf)
}
```

El bloque de 4096 bytes queda reservado hasta que el proceso termina. En un daemon (Bloque 3) que corre 24/7, esto acumula memoria hasta que el OOM Killer mata el proceso.

#### 2. Use-after-free — Usas memoria devuelta

```c
char *p = malloc(32);
free(p);
printf("%c\n", p[0]);  // BUG: p ya no te pertenece
```

Después de `free`, el allocator puede reutilizar esa memoria para otro `malloc`. Ahora `p[0]` lee datos de otra parte de tu programa. El resultado es impredecible y cambia entre ejecuciones.

#### 3. Double free — Devuelves dos veces

```c
char *p = malloc(32);
free(p);
free(p);  // BUG: corrompe las estructuras internas del allocator
```

El segundo `free` corrompe los metadatos internos de `malloc`. El programa puede seguir ejecutándose aparentemente bien y crashear mucho después en un `malloc` completamente inocente. **Patrón defensivo:** `free(p); p = NULL;` — `free(NULL)` está definido como no-op seguro.

#### 4. Out-of-bounds — Lees o escribes fuera de los límites

```c
int arr[4];
arr[4] = 42;  // BUG: solo arr[0]..arr[3] son válidos
```

C no verifica límites. Escribir fuera de un array en el stack sobreescribe variables vecinas o el return address de la función (esto es literalmente cómo funcionan los buffer overflow exploits).

### Arsenal de diagnóstico

| Herramienta | Qué detecta | Overhead | Cuándo usarla |
|-------------|-------------|----------|----------------|
| **ASan** | Out-of-bounds, use-after-free, double-free, leaks | ~2x | Tu herramienta **por defecto** durante desarrollo |
| **UBSan** | Signed overflow, null deref, misaligned access | ~1.1x | Siempre junto con ASan |
| **Valgrind** | Leaks (más preciso), accesos inválidos, uninitialized reads | ~20x | Auditoría final antes de commit |
| **GDB** | Inspección interactiva de estado, backtrace de crashes | 0 | Cuando necesitas entender el estado exacto en un punto |

> [!IMPORTANT]
> **ASan y Valgrind son mutuamente excluyentes.** Ambos interceptan `malloc`/`free`. Si compilas con `-fsanitize=address` y luego ejecutas con `valgrind`, obtendrás falsos positivos o crashes del propio Valgrind. Elige uno:
> - Compilar con ASan → ejecutar directamente
> - Compilar **sin** ASan (solo `-g -O0`) → ejecutar con `valgrind`

Flujo de trabajo recomendado:

```bash
# Desarrollo diario: ASan + UBSan (rápido)
gcc -g -O0 -fsanitize=address,undefined src/main.c -o build/main
./build/main

# Antes de commit: Valgrind (profundo)
gcc -g -O0 src/main.c -o build/main_valgrind
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./build/main_valgrind
```

### Leer la salida de Valgrind

```
==12345== 128 bytes in 1 blocks are definitely lost in loss record 1 of 1
==12345==    at 0x4C2FB0F: malloc (in /usr/lib/valgrind/...)
==12345==    by 0x10916A: process (main.c:12)
==12345==    by 0x1091B2: main (main.c:20)
```

Esto te dice: `malloc` fue llamado en `main.c:12` dentro de la función `process`, y ese bloque nunca fue liberado. El stacktrace te lleva directamente al punto donde deberías añadir `free`.

---

## Tema 4 — Docker: Laboratorio aislado y reproducible

### Por qué Docker y no tu máquina directamente

1. **Aislamiento:** Un `rm -rf /` accidental dentro del contenedor no destruye tu host.
2. **Reproducibilidad:** La imagen congela versiones exactas de `gcc`, `libc`, `valgrind`. Si tu ejercicio compila hoy, compilará dentro de 6 meses con la misma imagen.
3. **Dual-distro:** Este curso exige probar en Fedora **y** en Debian. Docker lo hace trivial.

### Dockerfile: la receta de tu entorno

Un Dockerfile para este curso sigue este patrón:

```dockerfile
# === Fedora ===
FROM fedora:latest
RUN dnf install -y gcc make gdb valgrind && dnf clean all
WORKDIR /app
COPY Makefile .
COPY src/ src/
RUN make all
CMD ["make", "run"]
```

```dockerfile
# === Debian ===
FROM debian:latest
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y gcc make gdb valgrind \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY Makefile .
COPY src/ src/
RUN make all
CMD ["make", "run"]
```

> [!NOTE]
> **`dnf clean all`** y **`rm -rf /var/lib/apt/lists/*`** eliminan la caché de paquetes. Sin esto, tus imágenes pesan cientos de MB innecesarios.

### Docker Compose: dos distros, un comando

```yaml
services:
  fedora:
    build:
      context: .
      dockerfile: Dockerfile.fedora
  debian:
    build:
      context: .
      dockerfile: Dockerfile.debian
```

```bash
# Construir y ejecutar ambas distros:
docker compose up --build

# Limpiar todo cuando termines:
docker system prune -f
```

### Debugging dentro de contenedores

GDB y `strace` necesitan la capability `SYS_PTRACE` que Docker deniega por defecto:

```bash
# Construir imagen con herramientas de debug
docker build -f Dockerfile.fedora -t debug-env .

# Ejecutar con ptrace habilitado
docker run --rm -it \
    --cap-add=SYS_PTRACE \
    --security-opt seccomp=unconfined \
    debug-env bash

# Dentro del contenedor:
gdb ./build/app
```

### Iteración rápida con bind mounts

Cuando estás escribiendo código, no quieres reconstruir la imagen Docker en cada cambio. Monta tu directorio local:

```bash
docker run --rm -it \
    -v "$PWD:/app" \
    -w /app \
    fedora:latest bash

# Dentro: instalar herramientas y compilar directamente
dnf install -y gcc make
make clean && make all && make run
```

Los cambios que hagas en tu editor se reflejan instantáneamente dentro del contenedor.

---

## Tema 5 — Configuración de programas: entorno, archivo y defaults

### Modelo de precedencia (el estándar de la industria)

Los programas Unix bien diseñados aceptan configuración de múltiples fuentes, donde cada una sobreescribe a la anterior:

```
Prioridad alta  ──▶  1. Flags de CLI        (--port 9090)
                     2. Variables de entorno (APP_PORT=9090)
                     3. Archivo de config    (config.env: APP_PORT=9090)
Prioridad baja  ──▶  4. Valores por defecto  (8080 hardcodeado)
```

En el Bloque 00 implementamos niveles 2, 3 y 4 (CLI se cubre en Bloque 01 con `getopt`).

### `getenv`: lo que debes saber

```c
const char *val = getenv("APP_PORT");
```

- Retorna `NULL` si la variable no existe.
- Retorna `""` (string vacío) si existe pero está vacía (`export APP_PORT=`).
- **El puntero retornado apunta a memoria del runtime.** No la modifiques. Copia siempre a un buffer propio.

> [!WARNING]
> `getenv` **no es thread-safe** en combinación con `setenv`/`putenv`. En un programa multihilo (Bloque 5), lee todas las variables de entorno al inicio, antes de crear threads.

### Parseo seguro de tipos

#### Strings: siempre copiar a buffer propio

```c
const char *raw = getenv("APP_ENV");
char env[64];
snprintf(env, sizeof(env), "%s", raw ? raw : "development");
// Ahora 'env' es tu copia segura, null-terminated, de tamaño controlado
```

#### Enteros: `strtol` con validación completa

```c
static long parse_port(const char *raw, long fallback) {
    if (!raw || *raw == '\0') return fallback;

    char *endptr;
    errno = 0;
    long val = strtol(raw, &endptr, 10);

    // Verificar: (1) consumió todo el string, (2) sin overflow, (3) rango válido
    if (*endptr != '\0' || errno == ERANGE || val < 1 || val > 65535) {
        fprintf(stderr, "Error: puerto inválido '%s' (debe ser 1-65535)\n", raw);
        return -1;  // Señalar error al caller
    }
    return val;
}
```

> [!CAUTION]
> **Nunca uses `atoi`.** `atoi("patata")` retorna `0` silenciosamente. `atoi("99999999999")` tiene comportamiento indefinido por overflow. `strtol` te da control total sobre errores.

#### Booleanos: normalización explícita

```c
static int parse_bool(const char *raw, int fallback) {
    if (!raw) return fallback;
    if (strcmp(raw, "1") == 0 || strcasecmp(raw, "true") == 0 ||
        strcasecmp(raw, "yes") == 0)
        return 1;
    if (strcmp(raw, "0") == 0 || strcasecmp(raw, "false") == 0 ||
        strcasecmp(raw, "no") == 0)
        return 0;
    fprintf(stderr, "Error: valor booleano inválido '%s'\n", raw);
    return -1;
}
```

### Fallback a archivo de configuración

Cuando la variable de entorno no existe, el programa intenta leer un archivo `config.env`:

```
# config.env
APP_PORT=8080
APP_ENV=development
APP_DEBUG=false
```

El parser lee línea por línea, ignora comentarios (`#`) y líneas vacías, y extrae pares `CLAVE=VALOR`. Esto es un ejercicio del bloque, no se da resuelto aquí, pero el contrato es claro:

1. Abrir archivo con `fopen`. Si no existe, usar defaults.
2. Leer líneas con `fgets`.
3. Ignorar líneas que empiezan con `#` o están vacías.
4. Separar por el primer `=`.
5. Stripear espacios/newlines del valor.

### Seguridad mínima en configuración

1. **Nunca loggees secretos completos.** Si tienes `API_KEY`, imprime `API_KEY=sk-****1234` (últimos 4 caracteres).
2. **No hardcodees secretos en el código ni en Dockerfiles.** Usa `docker run -e SECRET=...` o archivos `.env` excluidos de git.
3. **Distingue obligatorio vs opcional.** Si `APP_PORT` es obligatorio, falla ruidosamente al inicio si no está configurado. No esperes a que algo falle después.

---

## Ejercicios resueltos

### Toolchain

**T1-E1: Inspeccionar el preprocesado**

```bash
gcc -E src/main.c -o build/main.i
head -50 build/main.i
# Verás: cientos de líneas de headers expandidos antes de tu código
```

**T1-E2: Build manual multiarchivo**

```bash
gcc -Wall -Wextra -Werror -pedantic -std=c17 -c src/main.c -o build/main.o
gcc -Wall -Wextra -Werror -pedantic -std=c17 -c src/math.c -o build/math.o
gcc build/main.o build/math.o -o build/app
./build/app
```

Si olvidaras el segundo `.o`, obtendrías `undefined reference` — un error de enlazado.

**T1-E3: Comparar debug vs release**

```bash
gcc -g -O0    src/main.c -o build/debug
gcc -O2       src/main.c -o build/release
ls -lh build/debug build/release     # debug es más grande
nm build/debug | wc -l               # debug tiene más símbolos
strip build/release                  # strip elimina símbolos del release
```

### Memoria

**T3-E1: Leak y corrección**

```c
// BUG:
char *p = malloc(128);
strcpy(p, "hola");
return 0;  // leak: p nunca se libera

// FIX:
char *p = malloc(128);
if (!p) return 1;        // siempre verificar el retorno de malloc
strcpy(p, "hola");
free(p);
return 0;
```

```bash
valgrind --leak-check=full ./build/main
# Esperar: "All heap blocks were freed -- no leaks are possible"
```

**T3-E2: Double free y patrón defensivo**

```c
// BUG:
char *p = malloc(32);
free(p);
free(p);    // undefined behavior

// FIX:
char *p = malloc(32);
free(p);
p = NULL;   // free(NULL) es un no-op definido por el estándar
```

**T3-E3: Copia segura de variable de entorno**

```c
// BUG: modificar directamente el retorno de getenv
char *env = getenv("APP_ENV");
env[0] = toupper(env[0]);  // escritura ilegal sobre memoria del runtime

// FIX: copiar a buffer propio
const char *env = getenv("APP_ENV");
char buf[64];
snprintf(buf, sizeof(buf), "%s", env ? env : "dev");
buf[0] = toupper(buf[0]);  // ahora es seguro: buf es tuyo
```

---

## 3 Retos integradores (sin solución — son tu evaluación)

### Reto C1: Build matrix automática

Escribe `build-lab.sh` que:
1. Construya y testee un ejercicio en Fedora **y** Debian vía Docker.
2. Capture: versión de GCC, flags usados, resultado de tests, tiempo de ejecución.
3. Genere `report.json` con los resultados de ambas distros.
4. Salga con código `1` si cualquiera de las dos falla.

### Reto C2: Makefile con dependencias automáticas

Construye un `Makefile` que:
1. Soporte `make` (release), `make DEBUG=1` (debug+sanitizers), `make test`, `make clean`.
2. Use `-MMD -MP` para dependencias automáticas de headers.
3. Coloque todos los objetos en `build/` preservando estructura de subdirectorios.
4. **Criterio duro:** cambiar un `.h` recompila solo los `.c` que lo incluyen.

### Reto C3: Configuración tipada y segura

Desarrolla un loader de configuración en C:
1. Variables: `APP_PORT` (1–65535), `APP_ENV` (dev|staging|prod), `APP_DEBUG` (bool).
2. Precedencia: entorno → archivo `config.env` → defaults.
3. Mensajes de error claros por cada campo inválido.
4. **Tests de borde:** `APP_PORT=0`, `APP_PORT=abc`, `APP_PORT=99999`, `APP_ENV=qa`, `APP_DEBUG=maybe`.

---

## Checklist de salida del Bloque 00

Antes de avanzar al Bloque 01, verifica que puedes hacer **todo** esto:

- [ ] Compilar con flags completas (`-Wall -Wextra -Werror -pedantic -std=c17`) sin warnings
- [ ] Alternar entre perfil debug y release desde el Makefile
- [ ] Ejecutar `make test` y obtener resultados reproducibles
- [ ] Pasar el mismo ejercicio en Fedora y Debian via `docker compose up --build`
- [ ] Detectar un leak con Valgrind y corregirlo
- [ ] Detectar un out-of-bounds con ASan y corregirlo
- [ ] Leer configuración desde variables de entorno con validación de tipos y fallback

---

## Referencias

| Recurso | Comando |
|---------|---------|
| Compilador | `man gcc` |
| Make | `man make` |
| Memoria dinámica | `man 3 malloc`, `man 3 free`, `man 3 realloc` |
| Entorno | `man 3 getenv`, `man 3 setenv` |
| Valgrind | `man 1 valgrind` |
| Parseo numérico | `man 3 strtol` |
| Docker | [Dockerfile reference](https://docs.docker.com/engine/reference/builder/), [Compose reference](https://docs.docker.com/compose/compose-file/) |
