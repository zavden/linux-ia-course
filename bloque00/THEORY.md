# THEORY.md — Bloque 00: Entorno, Toolchain y Docker para C en Linux

Este bloque define la base técnica del curso. Si aquí quedan huecos, en los bloques siguientes (procesos, memoria compartida, sockets, seguridad) los errores se multiplican.

## Objetivos reales del bloque

Al terminar el Bloque 00 debes poder:

1. Explicar y controlar el pipeline de compilación de C (`preprocess -> compile -> assemble -> link`).
2. Mantener un `Makefile` profesional con perfiles de build, dependencias y tests.
3. Diagnosticar errores de memoria con herramientas correctas (`ASan`, `UBSan`, `Valgrind`, `GDB`).
4. Trabajar en entornos reproducibles Fedora/Debian con Docker y Compose.
5. Diseñar configuración robusta por variables de entorno y fallback por archivo.

---

## Tema 1: Toolchain C/GCC sin magia

## 1.1 Pipeline de compilación

`gcc` es un driver que coordina varias fases:

1. Preprocesado (`-E`): expande `#include`, macros y condicionales.
2. Compilación (`-S`): transforma C en ensamblador.
3. Ensamblado (`-c`): transforma ensamblador en objeto `.o`.
4. Enlazado (sin `-c`): resuelve símbolos y produce ejecutable o librería.

Comandos canónicos:

```bash
gcc -E main.c -o main.i
gcc -S main.i -o main.s
gcc -c main.s -o main.o
gcc main.o -o main
```

## 1.2 Warnings y estándar mínimo del curso

Flags base recomendadas:

```bash
-Wall -Wextra -Werror -pedantic -std=c17
```

Interpretación:

- `-Wall -Wextra`: activan warnings de calidad comunes.
- `-Werror`: transforma warning en error para evitar deuda técnica temprana.
- `-pedantic`: obliga a mantenerte en el estándar C.
- `-std=c17`: fija dialecto y evita sorpresas por defaults del compilador.

## 1.3 Debug vs Release

- Debug: `-g -O0 -fsanitize=address,undefined`
- Release: `-O2` (o `-O3` según perfil) sin sanitizers.

Regla práctica:

- Durante desarrollo: debug + sanitizers.
- Para benchmark final: release.

## 1.4 Linking estático y dinámico (base)

- Estático (`.a`): binario más independiente, tamaño mayor.
- Dinámico (`.so`): binario más pequeño, depende de librerías en runtime.

## 1.5 Errores típicos de toolchain

1. "undefined reference": faltó objeto/librería en el link.
2. "multiple definition": símbolo definido en más de un objeto.
3. Headers sin guardas (`#ifndef`) o declaraciones inconsistentes.
4. Build aparentemente "correcta" pero con ABI distinta por flags incompatibles.

---

## Ejercicios resueltos — Tema 1 (5)

### T1-E1: Inspeccionar preprocesado

Problema: verificar qué entrega el preprocesador.

Solución:

```bash
gcc -E src/main.c -o build/main.i
rg -n "printf|include" build/main.i | head
```

Verificación: `build/main.i` existe y contiene expansión de headers.

### T1-E2: Build manual multiarchivo

Problema: compilar `main.c` y `math.c` por separado y enlazar.

Solución:

```bash
gcc -Wall -Wextra -Werror -pedantic -std=c17 -c src/main.c -o build/main.o
gcc -Wall -Wextra -Werror -pedantic -std=c17 -c src/math.c -o build/math.o
gcc build/main.o build/math.o -o build/app
./build/app
```

Resultado esperado: ejecuta sin errores de link.

### T1-E3: Crear y usar librería estática

Problema: empaquetar utilidades en `libutil.a`.

Solución:

```bash
gcc -c src/str.c -o build/str.o
gcc -c src/io.c -o build/io.o
ar rcs build/libutil.a build/str.o build/io.o
gcc src/main.c -Lbuild -lutil -o build/app
```

Verificación: `build/libutil.a` existe y `app` corre.

### T1-E4: Compilar con sanitizers

Problema: detectar out-of-bounds en runtime.

Solución:

```bash
gcc -Wall -Wextra -Werror -pedantic -std=c17 -g -O0 \
  -fsanitize=address,undefined src/main.c -o build/app
./build/app
```

Verificación: ASan reporta stacktrace del acceso inválido.

### T1-E5: Comparar binario debug vs release

Problema: observar impacto de `-g/-O0` frente a `-O2`.

Solución:

```bash
gcc -g -O0 src/main.c -o build/app_debug
gcc -O2 src/main.c -o build/app_release
ls -lh build/app_debug build/app_release
nm build/app_debug | head
```

Verificación: `app_debug` contiene más símbolos; tamaños distintos.

---

## Tema 2: GNU Make bien hecho

## 2.1 Modelo mental correcto

`make` no es "script runner" únicamente. Es un resolvedor de dependencias basado en timestamps.

Si una dependencia cambia, recompila solo lo necesario.

## 2.2 Estructura mínima profesional

```makefile
CC := gcc
CFLAGS := -Wall -Wextra -Werror -pedantic -std=c17
SRC := $(wildcard src/*.c)
OBJ := $(SRC:src/%.c=build/%.o)
TARGET := build/main

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build

.PHONY: all clean
```

## 2.3 Conceptos que debes dominar

- Variables (`:=`, `?=`, `+=`).
- Automáticas (`$@`, `$<`, `$^`).
- `.PHONY`.
- Reglas patrón (`%.o: %.c`).
- Inclusión de dependencias automáticas (`-MMD -MP`).

## 2.4 Error clásico

Usar espacios en lugar de TAB en recetas. Make falla con: `missing separator`.

---

## Ejercicios resueltos — Tema 2 (5)

### T2-E1: Targets base (`all`, `run`, `test`, `clean`)

Problema: definir flujo mínimo del ejercicio.

Solución:

```makefile
.PHONY: all run test clean
all: build/main
run: build/main
	./build/main
test: build/main
	bash tests/test.sh
clean:
	rm -rf build
```

Verificación: `make run` ejecuta y `make clean` limpia.

### T2-E2: Compilación incremental

Problema: evitar recompilar todo al cambiar un archivo.

Solución: usar objetos por archivo + regla patrón (como en 2.2).

Verificación: modificar `src/math.c` recompila solo `build/math.o` y relinka.

### T2-E3: Dependencias de headers automáticas

Problema: recompilar cuando cambia `.h`.

Solución:

```makefile
CFLAGS += -MMD -MP
-include $(OBJ:.o=.d)
```

Verificación: cambiar `src/math.h` dispara recompilación de objetos dependientes.

### T2-E4: Perfil debug opcional

Problema: alternar debug/release sin duplicar reglas.

Solución:

```makefile
ifdef DEBUG
CFLAGS += -g -O0 -fsanitize=address,undefined
LDFLAGS += -fsanitize=address,undefined
else
CFLAGS += -O2
endif
```

Uso:

```bash
make clean && make DEBUG=1
```

### T2-E5: Test matrix local dual-distro

Problema: ejecutar tests del mismo código en Fedora y Debian.

Solución:

```bash
docker build -f Dockerfile.fedora -t b00-fedora .
docker run --rm b00-fedora make test

docker build -f Dockerfile.debian -t b00-debian .
docker run --rm b00-debian make test
```

Verificación: ambos pasan sin tocar código host.

---

## Tema 3: Memoria en C y diagnóstico serio

## 3.1 Modelo de memoria del proceso

Mapa conceptual (simplificado):

1. `text`: código.
2. `rodata`: constantes de solo lectura.
3. `data`/`bss`: globales inicializadas/no inicializadas.
4. `heap`: asignación dinámica (`malloc`, `calloc`, `realloc`, `free`).
5. `stack`: frames de funciones, variables automáticas.

## 3.2 Reglas de oro

1. Cada `malloc/calloc/realloc` exitoso debe tener un `free` coherente.
2. Nunca usar memoria después de `free` (use-after-free).
3. Nunca liberar dos veces el mismo puntero (double free).
4. Nunca escribir fuera de límites de arrays.
5. No asumir que variables no inicializadas tienen valor conocido.

## 3.3 Herramientas y cuándo usarlas

- `ASan`: rápido, excelente para desbordes y UAF.
- `UBSan`: UB (overflows signed, alineación, etc.).
- `Valgrind`: leak analysis profundo y accesos inválidos, más lento.
- `GDB`: inspección interactiva de estado y backtrace.

Comando típico Valgrind:

```bash
valgrind --leak-check=full --show-leak-kinds=all ./build/main
```

## 3.4 `getenv` sin mitos

- `getenv("X")` devuelve puntero a memoria gestionada por el runtime.
- No debes modificar ese buffer.
- Si necesitas transformarlo, copia a buffer propio (`strncpy`/`snprintf`).

---

## Ejercicios resueltos — Tema 3 (5)

### T3-E1: Leak básico y fix

Problema:

```c
char *p = malloc(128);
strcpy(p, "hola");
return 0;
```

Solución:

```c
char *p = malloc(128);
if (!p) return 1;
strcpy(p, "hola");
free(p);
return 0;
```

Verificación:

```bash
valgrind --leak-check=full ./build/main
```

Debe reportar `definitely lost: 0 bytes`.

### T3-E2: Out-of-bounds en stack

Problema:

```c
int a[4];
a[4] = 99;
```

Solución: usar índice válido `0..3`.

Verificación con ASan:

```bash
gcc -g -O0 -fsanitize=address src/main.c -o build/main
./build/main
```

### T3-E3: Double free

Problema:

```c
char *p = malloc(32);
free(p);
free(p);
```

Solución:

```c
char *p = malloc(32);
free(p);
p = NULL;
```

Verificación: ASan/Valgrind sin errores.

### T3-E4: Use-after-free

Problema:

```c
char *p = malloc(32);
free(p);
p[0] = 'A';
```

Solución: no acceder tras liberar; re-asignar memoria si se necesita.

### T3-E5: Copia segura de variable de entorno

Problema: mutar directamente el retorno de `getenv`.

Solución:

```c
const char *env = getenv("APP_ENV");
char app_env[32];
snprintf(app_env, sizeof(app_env), "%s", env ? env : "development");
```

Verificación: no hay escritura sobre memoria de entorno compartida.

---

## Tema 4: Docker como laboratorio reproducible

## 4.1 Principios de uso en este curso

1. El host no es laboratorio; el contenedor sí.
2. Misma fuente, dos distros: Fedora y Debian.
3. Build reproducible: comandos explícitos, dependencias declaradas.

## 4.2 Diferencias clave Dockerfile vs Compose

- `Dockerfile`: define imagen.
- `docker compose`: orquesta múltiples servicios y configuración compartida.

## 4.3 Buenas prácticas mínimas

1. No depender de estado manual del contenedor.
2. Limpiar caché de paquetes (`dnf clean all`, `rm -rf /var/lib/apt/lists/*`).
3. Mantener `WORKDIR` consistente.
4. Evitar privilegios extra salvo necesidad técnica real.

## 4.4 Debugging en contenedor

- `gdb/strace` puede requerir:

```bash
docker run --rm -it --cap-add=SYS_PTRACE --security-opt seccomp=unconfined imagen
```

---

## Ejercicios resueltos — Tema 4 (5)

### T4-E1: Dockerfile mínimo compilable

Problema: construir y ejecutar `hello.c`.

Solución Fedora:

```dockerfile
FROM fedora:latest
RUN dnf install -y gcc make && dnf clean all
WORKDIR /app
COPY src/ ./src/
COPY Makefile .
RUN make all
CMD ["make", "run"]
```

Verificación:

```bash
docker build -f Dockerfile.fedora -t b00-e1-fedora .
docker run --rm b00-e1-fedora
```

### T4-E2: Build Debian equivalente

Problema: asegurar portabilidad de build.

Solución Debian:

```dockerfile
FROM debian:latest
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y gcc make && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY src/ ./src/
COPY Makefile .
RUN make all
CMD ["make", "run"]
```

Verificación: output funcional equivalente en Debian.

### T4-E3: Iteración rápida con bind mount

Problema: editar host y compilar dentro del contenedor.

Solución:

```bash
docker run --rm -it -v "$PWD":/app -w /app debian:latest bash
apt-get update && apt-get install -y gcc make
make all && make run
```

Resultado: cambios reflejados al instante sin rebuild de imagen.

### T4-E4: Compose dual-distro

Problema: ejecutar el mismo ejercicio en dos servicios.

Solución:

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

Verificación:

```bash
docker compose up --build
```

### T4-E5: GDB en contenedor

Problema: depurar crash dentro de Docker.

Solución:

```bash
docker build -f Dockerfile.debian -t b00-debug .
docker run --rm -it --cap-add=SYS_PTRACE --security-opt seccomp=unconfined b00-debug bash
gdb ./build/main
```

Verificación: backtrace disponible dentro del contenedor.

---

## Tema 5: Configuración por entorno y fallback

## 5.1 Modelo recomendado de precedencia

1. CLI (si existe)
2. Variables de entorno
3. Archivo de configuración
4. Defaults internos

Para Bloque 00 (simple):

1. Variables de entorno
2. Archivo
3. Defaults

## 5.2 Parseo robusto

- Enteros: `strtol` + validación de rango y errores.
- Booleanos: normalizar (`true/false`, `1/0`, `yes/no`).
- Strings: `snprintf` con buffer fijo.

## 5.3 Seguridad mínima

1. Nunca imprimir secretos completos en logs.
2. No hardcodear tokens/passwords en código ni en imágenes.
3. Distinguir variables obligatorias vs opcionales.

---

## Ejercicios resueltos — Tema 5 (5)

### T5-E1: Lectura básica con defaults

Problema: leer `APP_PORT` y `APP_ENV` con valores por defecto.

Solución (C):

```c
const char *port_s = getenv("APP_PORT");
const char *env_s  = getenv("APP_ENV");
printf("PORT=%s\n", port_s ? port_s : "8080");
printf("ENV=%s\n", env_s ? env_s : "development");
```

### T5-E2: Entero válido con `strtol`

Problema: validar puerto y rango.

Solución (C):

```c
char *end = NULL;
long port = strtol(port_s ? port_s : "8080", &end, 10);
if (*end != '\0' || port < 1 || port > 65535) {
    fprintf(stderr, "APP_PORT invalido\n");
    return 1;
}
```

### T5-E3: Booleano robusto

Problema: parsear `APP_DEBUG` (`1/0`, `true/false`, `yes/no`).

Solución: normalizar string a minúsculas y mapear conjunto permitido.

Resultado esperado: entrada inválida produce error explícito.

### T5-E4: Fallback a archivo

Problema: si no hay `APP_ENV`, leer `config.env`.

Solución (estrategia):

1. Intentar `getenv`.
2. Si `NULL`, abrir `config.env` y buscar clave `APP_ENV=`.
3. Si no aparece, usar default.

Verificación: cambiar solo archivo modifica comportamiento.

### T5-E5: Inyección con Compose

Problema: configurar runtime sin recompilar.

Solución (`docker-compose.yml`):

```yaml
services:
  app:
    build: .
    environment:
      APP_PORT: "9090"
      APP_ENV: "staging"
```

Verificación: `docker compose up --build` imprime valores inyectados.

---

## 3 ejercicios complicados (para estudiante)

Estos no van resueltos; son integradores del Bloque 00.

### Reto C1: Build matrix automática con reporte

Implementa `build-lab.sh` que:

1. Haga build y test de un ejercicio en Fedora y Debian.
2. Capture: versión de `gcc`, flags usados, resultado de tests y tiempo de ejecución.
3. Genere `report.json` consolidado.

Criterio duro: script falla (`exit 1`) si cualquier distro falla.

### Reto C2: Makefile con dependencias automáticas y perfiles

Construye un `Makefile` que soporte:

1. `make` (release), `make DEBUG=1`, `make test`, `make clean`.
2. Dependencias `.d` automáticas.
3. Objetos en `build/` manteniendo estructura de subdirectorios.

Criterio duro: modificar un `.h` recompila solo lo necesario.

### Reto C3: Configuración tipada y segura

Desarrolla un loader de configuración en C con:

1. `APP_PORT` (1..65535), `APP_ENV` (`dev|staging|prod`), `APP_DEBUG` bool.
2. Precedencia entorno > archivo > default.
3. Mensajes de error detallados por campo inválido.

Criterio duro: tests de borde (`APP_PORT=0`, `APP_PORT=abc`, `APP_ENV=qa`) deben fallar con salida clara.

---

## Checklist de salida del Bloque 00

1. Compilo con `-Wall -Wextra -Werror -pedantic -std=c17`.
2. Tengo flujo debug/release controlado.
3. `make test` es confiable y repetible.
4. Docker Fedora y Debian pasan el mismo ejercicio.
5. Variables de entorno están validadas y con defaults explícitos.
6. Puedo explicar y demostrar al menos 3 errores de memoria comunes y cómo detectarlos.

---

## Referencias técnicas recomendadas

1. `man gcc`
2. `man make`
3. `man 3 malloc`, `man 3 free`, `man 3 getenv`
4. `man 1 valgrind`
5. Docker docs: Dockerfile reference y Compose reference

