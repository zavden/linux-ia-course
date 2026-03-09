# 📖 THEORY_GEMINI.md — Bloque 00: Entorno, Toolchain y Docker para C en Linux

> [!IMPORTANT]
> **Base Operativa del Curso:** Este bloque define tu entorno de trabajo. Si aquí quedan lagunas técnicas (como no entender un Makefile o ignorar un *Memory Leak*), en los bloques siguientes de concurrencia y sockets, los errores se multiplicarán críticamente volviéndose imposibles de rastrear.

---

## 🎯 Objetivos Reales del Bloque

Al terminar el Bloque 00 debes estar capacitado para:

1. **Controlar el pipeline de C sin magia** (`Preprocesado -> Compilación -> Ensamblado -> Enlazado`).
2. **Escribir un `Makefile` profesional** que entienda dependencias, perfiles (Debug/Release) y evite recompilaciones inútiles.
3. **Diagnosticar la memoria sin adivinar**, utilizando eficientemente `ASan`, `UBSan` y `Valgrind`.
4. **Dominar Docker como laboratorio**, ejecutando tu código en entornos Fedora/Debian aislados e idénticos.
5. **Configurar aplicaciones robustas** mediante variables de entorno y archivos de *fallback*.

---

## 1. ⚙️ Toolchain C y GCC sin Magia

### 1.1 El Pipeline de Compilación paso a paso

Cuando ejecutas `gcc main.c`, internamente suceden 4 fases secuenciales. Entenderlas es vital para depurar errores esotéricos:

1. **Preprocesado (`-E`):** Resuelve los `#include` (pega el código de las cabeceras), expande macros (`#define`) y evalúa condicionales (`#ifdef`). *Genera código C puro*.
2. **Compilación (`-S`):** Analiza la sintaxis del código puro y lo traduce a código Ensamblador específico de tu CPU. 
3. **Ensamblado (`-c`):** Convierte el ensamblador a código máquina binario (archivos Objeto `.o`). Aún no es ejecutable porque faltan las direcciones de las librerías externas (como `printf`).
4. **Enlazado (Linker):** Toma todos los `.o` y las librerías dinámicas/estáticas (`.so`/`.a`), resuelve las firmas que faltaban y emite el Ejecutable final.

### 1.2 El Estándar de Calidad (Tus Flags de Compilación)

En este curso, compilaremos con un nivel de paranoia alto para evitar arrastrar "Deuda Técnica":

```bash
gcc -Wall -Wextra -Werror -pedantic -std=c17 src/main.c
```

- `-Wall -Wextra`: Activa casi todos los warnings útiles del compilador.
- `-Werror`: **Obliga** a que cualquier warning aborte la compilación (Warning = Error).
- `-pedantic`: Rechaza extensiones propietarias de GCC; asegura que tu código sea C estándar real.
- `-std=c17`: Fija implícitamente la versión del lenguaje a usar.

### 1.3 Perfiles: Debug vs Release

Al desarrollar, quieres información; al desplegar a producción, quieres velocidad.

> [!TIP]
> **Desarrollo (Debug):** `-g -O0 -fsanitize=address,undefined`
> Deja intactos los símbolos para GDB (`-g`), desactiva optimizaciones que reordenan código (`-O0`) e inyecta sanitizadores que detendrán el programa si haces algo ilegal (como escribir fuera de un array).
> 
> **Producción (Release):** `-O2` o `-O3` sin sanitizadores.
> El compilador puede desenrollar bucles e ignorar código muerto para máxima eficiencia.

---

## 2. 🐘 GNU Make Profesional

Un `Makefile` no es un simple script de Bash enmascarado (`run.sh`). Es un motor de **resolución de grafos de dependencias** basado en fechas de modificación (timestamps).

### 2.1 La Anatomía Correcta

Un Makefile decente no compila "todo siempre". Recompila solo los `.o` (objetos) cuyo código fuente asociado (`.c` o `.h`) sea más nuevo que ellos.

```makefile
CC := gcc
CFLAGS := -Wall -Wextra -Werror -pedantic -std=c17
SRC := $(wildcard src/*.c)
# Transforma la lista 'src/main.c src/math.c' en 'build/main.o build/math.o'
OBJ := $(SRC:src/%.c=build/%.o)
TARGET := build/app

# Target por defecto
all: $(TARGET)

# Enlaza los objetos si el TARGET es más viejo que alguno de los OBJ
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@

# Regla Patrón: Compila un .o solo si su .c correspondiente cambió
build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build

# Marca targets que NO son archivos físicos
.PHONY: all clean
```

> [!CAUTION]
> **El Error de los Espacios:** En los Makefiles, la acción debajo de una regla (ej. el `$(CC) ...`) **DEBE** estar indentada por un bloque `TAB` real, nunca con espacios. Si usas espacios verás el infame error `missing separator`.

---

## 3. 🧠 Memoria en C y Diagnóstico Serio

El paradigma fundamental de C es que **tú eres el Garbarge Collector**. 

### 3.1 Las 5 Reglas de Oro de Memoria Dinámica
1. Por cada `malloc/calloc` exitoso en tu programa, debe existir un único `free` alcanzable (Ownership).
2. **Use-After-Free:** Si haces `free(p)`, ese puntero muere. Volver a leer `p[0]` causará corrupción de memoria.
3. **Double-Free:** Llamar `free(p)` dos veces sobre la misma dirección abortará el programa violentamente.
4. **Out-of-Bounds:** C no comprueba los límites del array. Escribir en `array[10]` si el tamaño es `5` pisotea variables vecinas (Stack/Heap Overflow).
5. Las variables locales no se inicializan solas. Sumar sobre `int foo;` suma sobre basura residual de la RAM apuntada.

### 3.2 ¿Qué herramienta usar?

Tu arsenal de diagnóstico se divide según la necesidad:

| Herramienta | Cuándo usarla | Cómo invocarla |
|-------------|---------------|----------------|
| **ASan** (Address Sanitizer) | Desarrollo rápido. Frena en secot crashes y desbordes. | Compilar con: `-fsanitize=address` |
| **UBSan** (UB Sanitizer) | Detecta *Comportamiento indefinido* silencioso (ej. desbordar un entero con signo). | Compilar con: `-fsanitize=undefined` |
| **Valgrind** | Auditoría final. Caza minuciosamente "Memory Leaks". Es lento por ser un emulador. | Ejecutar binario limpio con: `valgrind --leak-check=full ./app` |
| **GDB** | Para *Post-mortem* o avanzar línea a línea viendo variables y registros. | `gdb ./app` |

> [!WARNING]
> Nunca uses **Valgrind** inspeccionando un binario que a la vez fue compilado con **ASan**. Entrarán en conflicto por la intercepción de `malloc()`.

---

## 4. 🐳 Docker como Laboratorio Reproducible

En Linux de Sistemas no "ensuciamos nuestro entorno Host" para probar dependencias extrañas.

### 4.1 Principios de uso
1. Tu Mac/PC es solo el editor. **El contenedor es el laboratorio.**
2. Comprobamos la portabilidad: Tu código debe compilar idéntico usando *DNF* (Fedora) y *APT* (Debian).

### 4.2 Ejecución Dual con Docker Compose

En vez de ejecutar comandos inabarcables, un `docker-compose.yml` modela este entorno paralelo.

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

Solo debes tirar un `docker compose up --build` y observarás cómo las dos distribuciones levantan, compilan tu código usando los Makefiles que creaste y pasan los tests de Bash, todo a la vez al lado de tu código fuente.

---

## 5. 🌍 Configuración Robusta (Variables de Entorno)

Los programas de consola en Unix deben configurarse dinámicamente, no modificando código fuente. La jerarquía de configuración estándar es:

1. Modificadores de terminal (`--port 8080`).
2. **Variables de entorno** (`APP_PORT=8080`).
3. Archivos locales de configuración (fallback a un `config.env`).
4. Valores predeterminados en el Código (Default).

### El mito del "Modifica `getenv()` directamnete"

> [!CAUTION]
> La función `getenv("VAR_NAME")` te devuelve un puntero que apunta a un área de memoria sagrada gestionada por la libreria C. **¡Nunca modifiques esa memoria!** Si necesitas cambiarla o "limpiarla", cópiala primero a un buffer propio:

```c
const char *raw_env = getenv("APP_ENV");
char app_env[32];

// Fallback elegante (operador ternario incrustado en memoria local)
snprintf(app_env, sizeof(app_env), "%s", raw_env ? raw_env : "development");
```

### Parseo Seguro Numérico con Fallback
Como aprendimos en el Bloque 01, siempre que traigas un número del Entorno, trátalo con `strtol`.

```c
const char *port_raw = getenv("APP_PORT");
char *endptr;
long port = strtol(port_raw ? port_raw : "8080", &endptr, 10);

if (*endptr != '\0' || port < 1 || port > 65535) {
    fprintf(stderr, "FATAL: Puerto HTTP inválido o fuera de rango (1-65535).\n");
    exit(EXIT_FAILURE);
}
```

---

### 📚 Entregables Finales Esperados del Estudiante
1. Estructura de `Makefile` que separa el código en la carpeta `build/` compilado independientemente.
2. Contenedores de prueba (`Dockerfiles` fedora y debian) que inyecten variables simuladas (`ENV`) al sistema.
3. El programa que demuestre que `valgrind --leak-check=full` marca 0 bytes perdidos a la hora de limpiar toda la configuración leída.
