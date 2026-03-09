# ==============================================================================
# Ejercicio 0.2 — Makefile Básico (SOLUCIÓN DIDÁCTICA)
# 
# OBJETIVO DIDÁCTICO:
# Este Makefile demuestra las mejores prácticas para proyectos en C:
# compilación separada (generar .o y luego enlazar), uso de variables de entorno,
# directivas .PHONY, auto-generación de dependencias (.d) y buid fuera de source ("out-of-tree").
# ==============================================================================

# ------------------------------------------------------------------------------
# 1. VARIABLES DEL COMPILADOR Y FLAGS
# ------------------------------------------------------------------------------
CC       = gcc

# CFLAGS: Flags para el compilador de C.
# -Wall -Wextra -Werror: Activan advertencias rigurosas y las tratan como errores. (Seguridad)
# -pedantic: Exige C estándar puro, sin extensiones raras del compilador.
# -std=c17: Especifica el estándar del lenguaje C17.
# -MMD -MP: Instruyen a GCC para que genere archivos .d que contienen las dependencias
#           de los headers (.h) automáticamente. Así Make sabe si modificaste un header.
CFLAGS   = -Wall -Wextra -Werror -pedantic -std=c17 -MMD -MP

# LDFLAGS: Flags para la fase de enlazado (Linking). (Vacío por defecto).
LDFLAGS  =
# LDLIBS: Bibliotecas a enlazar (ej: -lm para math.h, -lpthread).
LDLIBS   =

# ------------------------------------------------------------------------------
# 2. DEFINICIÓN DE ESTRUCTURA Y ARCHIVOS
# ------------------------------------------------------------------------------
SRC_DIR  = src
BUILD_DIR = build

# $(wildcard src/*.c) busca todos los archivos que terminen en .c en la carpeta src.
# SRCS contendrá algo como: src/main.c src/greet.c src/info.c
SRCS     = $(wildcard $(SRC_DIR)/*.c)

# $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o) es una sustitución de patrones.
# Le dice a Make: Toma la lista SRCS y convierte "src/algo.c" en "build/algo.o".
OBJS     = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Igual que arriba, generamos la lista de archivos de dependencias reemplazando .o por .d
DEPS     = $(OBJS:.o=.d)

# El ejecutable final a generar
TARGET   = $(BUILD_DIR)/main

# ------------------------------------------------------------------------------
# 3. VARIABLES CONDICIONALES (Modo Debug vs Release)
# ------------------------------------------------------------------------------
# Si el usuario ejecuta "make DEBUG=1", esta condición será verdadera.
ifdef DEBUG
  # -g: Añade información de depuración para GDB
  # -O0: Desactiva optimizaciones para un debug predecible.
  # -fsanitize=... : Activa detectores de fugas de memoria y comportamiento indefinido en runtime.
  CFLAGS += -g -O0 -fsanitize=address,undefined
  LDFLAGS += -fsanitize=address,undefined
else
  # -O2: Optimización de nivel 2. (Modo producción/Release).
  CFLAGS += -O2
endif

# ------------------------------------------------------------------------------
# 4. REGLAS PRINCIPALES (Targets)
# ------------------------------------------------------------------------------

# .PHONY declara que estos "nombres" no son archivos físicos que deban construirse.
# Evita un conflicto si accidentalmente crearas un archivo llamado "clean" o "test".
.PHONY: all clean run test debug

# "all" es por convención la regla que se ejecuta si llamas a "make" sin argumentos.
# Depende del $(TARGET) (el binario principal).
all: $(TARGET)

# ------------------------------------------------------------------------------
# 5. REGLAS DE CONSTRUCCIÓN (Cómo fabricar las cosas)
# ------------------------------------------------------------------------------

# REGLA DE ENLACE (Linkeo):
# Depende de de todos los $(OBJS) (.o) y también del directorio de build `| $(BUILD_DIR)`
# El pipe '|' indica una dependencia de solo-orden: "el directorio debe existir antes", 
# pero no recompilemos solo porque el timestamp del directorio cambió.
# $@ = El nombre del target (build/main)
# $^ = Todos los prerrequisitos (build/main.o build/greet.o build/info.o)
$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# REGLA DE COMPILACIÓN (Patrón %):
# Define cómo construir CUALQUIER .o a partir de CUALQUIER .c
# $< = La primera dependencia (el archivo .c)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Regla simple para crear el directorio de salida si no existe
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# ------------------------------------------------------------------------------
# 6. REGLAS DE UTILIDAD
# ------------------------------------------------------------------------------

# Borra agresivamente (-rf) la carpeta de compilación generada.
clean:
	rm -rf $(BUILD_DIR)

# Atajo para compilar (dependencia de $(TARGET)) y luego, si tuvo éxito, ejecutarlo.
run: $(TARGET)
	./$(TARGET)

# Ejecuta el mismo make, pero inyectando la variable DEBUG=1
debug:
	$(MAKE) DEBUG=1 all

# Si existe un directorio tests, itera sobre los .sh y los ejecuta usando bash.
# El condicional "@if" está prefijado con "@", por lo qu Make no imprimirá el comando literal antes de correrlo.
test: $(TARGET)
	@echo "=== Ejecutando tests ==="
	@if [ -d tests ]; then \
		for t in tests/*.sh; do \
			bash "$$t" || exit 1; \
		done; \
	fi

# ------------------------------------------------------------------------------
# 7. INCLUSIÓN DE DEPENDENCIAS AUTOGENERADAS
# ------------------------------------------------------------------------------
# Incluimos los archivos .d (que GCC genera gracias a -MMD).
# El guion "-" al principio le dice a Make: "no te quejes con un error si los .d aún no existen" 
# (porque obviamente en la primera compilación limpia, aún no existirán).
-include $(DEPS)
