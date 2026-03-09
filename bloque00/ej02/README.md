# Ejercicio 0.2 — Makefile Básico

## 🎯 Objetivo

Dominar **GNU Make** como sistema de build para C. Entender reglas, targets, variables, y patrones. Crear un Makefile profesional que usarás como base en todo el curso.

## 📚 Teoría — GNU Make desde Cero

### ¿Qué es Make?

Make es un **sistema de build** que automatiza la compilación. Lee un archivo llamado `Makefile` con **reglas** que describen cómo construir tu proyecto.

### Anatomía de una Regla

```makefile
target: dependencias
	receta (comando)
```

- **target**: lo que quieres construir (un archivo o un nombre simbólico).
- **dependencias**: archivos que deben existir antes de ejecutar la receta.
- **receta**: comandos de shell. **DEBEN empezar con TAB, no espacios.**

```makefile
main: main.c
	gcc -o main main.c
```

Esto dice: "Para construir `main`, necesito `main.c`. El comando es `gcc`."

### Variables

```makefile
CC      = gcc                    # Compilador
CFLAGS  = -Wall -Wextra -std=c17 # Flags de compilación
LDFLAGS =                        # Flags del linker
LDLIBS  = -lm                    # Bibliotecas

main: main.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o main main.c $(LDLIBS)
```

**Variables automáticas** (dentro de recetas):

| Variable | Significado | Ejemplo |
|----------|-------------|---------|
| `$@` | El target actual | `main` |
| `$<` | La primera dependencia | `main.c` |
| `$^` | Todas las dependencias | `main.c utils.c` |
| `$*` | El "stem" del patrón | `main` (de `main.o`) |

### Compilación Separada

En C real, no compilas todo de una vez. Separas en dos fases:

```
.c → .o (compilación)    gcc -c -o main.o main.c
.o → binario (linkeo)    gcc -o main main.o utils.o
```

**¿Por qué?** Si cambias un solo `.c`, solo recompilas ese `.o`, no todo.

### Reglas de Patrón (Pattern Rules)

```makefile
# Regla genérica: cualquier .o se construye desde su .c
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
```

Esto reemplaza escribir una regla por cada archivo.

### .PHONY

Los targets que no son archivos deben declararse como `.PHONY`:

```makefile
.PHONY: all clean run test

all: main        # No es un archivo, es un comando
clean:           # No es un archivo
	rm -f *.o main
```

Sin `.PHONY`, si existe un archivo llamado `clean`, Make pensará que ya está actualizado.

### Funciones Útiles

```makefile
SRCS = $(wildcard src/*.c)            # Todos los .c en src/
OBJS = $(SRCS:src/%.c=build/%.o)      # Reemplazar src/*.c → build/*.o
DEPS = $(OBJS:.o=.d)                  # Reemplazar *.o → *.d
```

### Dependencias Automáticas

GCC puede generar archivos `.d` con las dependencias de cada `.c`:

```makefile
CFLAGS += -MMD -MP          # Generar .d con cada compilación

-include $(DEPS)            # Incluir si existen (el - ignora errores)
```

Esto significa que si cambias un `.h`, Make sabe qué `.c` recompilar.

### Variables Condicionales

```makefile
ifdef DEBUG
  CFLAGS += -g -O0 -fsanitize=address,undefined
  LDFLAGS += -fsanitize=address,undefined
else
  CFLAGS += -O2
endif
```

Uso: `make DEBUG=1`

### Resumen Visual

```
Makefile
├── Variables (CC, CFLAGS, SRCS, OBJS...)
├── .PHONY targets (all, clean, run, test)
├── Target principal (all: programa)
├── Regla de linkeo (programa: objs → gcc -o)
├── Regla de patrón (%.o: %.c → gcc -c)
├── Creación de directorios (build/)
├── Phony targets (clean, run, test, debug)
└── -include $(DEPS)
```

## 📝 Instrucciones

1. **Crea un programa multi-archivo:**
   - `src/main.c` — función `main()` que llama a `greet()` y `info()`.
   - `src/greet.c` + `src/greet.h` — función `greet(const char *name)`.
   - `src/info.c` + `src/info.h` — función `info()` que imprime info del compilador.

2. **Escribe el Makefile** con estos requisitos:
   - Variables: `CC`, `CFLAGS`, `LDFLAGS`, `LDLIBS`, `SRC_DIR`, `BUILD_DIR`.
   - `CFLAGS` debe incluir: `-Wall -Wextra -Werror -pedantic -std=c17`.
   - **Compilación separada**: `.c` → `.o` → binario.
   - **Auto-detección de fuentes**: usar `wildcard`.
   - Targets `.PHONY`: `all`, `clean`, `run`, `test`, `debug`.
   - `debug` target: recompila con `-g -O0 -fsanitize=address,undefined`.
   - **Build directory**: objetos en `build/`, no en `src/`.
   - **Dependencias automáticas**: `-MMD -MP` + `-include`.

3. **Verifica que funciona:**
   ```bash
   make              # Compila en build/
   make run          # Ejecuta
   make clean        # Limpia build/
   make DEBUG=1      # Compila con ASan
   make test         # Ejecuta tests
   ```

## ✅ Criterios de Éxito

- [ ] `make` compila 3 archivos `.c` → 3 `.o` → 1 binario, 0 warnings.
- [ ] `make clean && make` recompila todo.
- [ ] Modificar solo `greet.c` y hacer `make` solo recompila `greet.o` (no todo).
- [ ] `make DEBUG=1` activa ASan y `-g`.
- [ ] `build/` contiene: `.o`, `.d`, y el binario. `src/` queda limpio.
- [ ] Tests pasan en Fedora y Debian.

## 💡 Pistas

<details>
<summary>Pista 1 — Estructura del Makefile</summary>

```makefile
CC       = gcc
CFLAGS   = -Wall -Wextra -Werror -pedantic -std=c17 -MMD -MP
SRC_DIR  = src
BUILD_DIR = build
SRCS     = $(wildcard $(SRC_DIR)/*.c)
OBJS     = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS     = $(OBJS:.o=.d)
TARGET   = $(BUILD_DIR)/main

# ... reglas aquí ...

-include $(DEPS)
```
</details>

<details>
<summary>Pista 2 — info() con macros del compilador</summary>

GCC define macros que puedes usar:
```c
printf("Compilador: %s\n", __VERSION__);
printf("Fecha: %s %s\n", __DATE__, __TIME__);
printf("Estándar C: %ld\n", __STDC_VERSION__);
printf("Archivo: %s, Línea: %d\n", __FILE__, __LINE__);
```
</details>

## 📖 Referencias

- [GNU Make Manual](https://www.gnu.org/software/make/manual/)
- `man gcc` — buscar `-Wall`, `-Wextra`, `-fsanitize`
- `man make`
