# Ejercicio C.3 — Makefiles de Nivel Intermedio

## 🎯 Objetivo
Hacer que tu Makefile sea **genérico**. En un proyecto de 50 archivos, no puedes escribir 50 reglas manualmente.

## 📚 Teoría Mínima
### Variables
```makefile
CC = gcc
CFLAGS = -Wall -Wextra
```
Se usan con `$()`, por ejemplo `$(CC)`.

### Variables Automáticas en Reglas
Dentro de una regla, Make define variables mágicas:
- `$@`: El nombre del objetivo (lo de la izquierda de los `:`).
- `$<`: El nombre de la primera dependencia.
- `$^`: Todas las dependencias.

### Reglas de Patrón (Pattern Rules)
Le dicen a Make cómo construir *cualquier* archivo de un tipo a partir de otro.
```makefile
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
```
Significado: Para construir `cualquier_cosa.o` a partir de `cualquier_cosa.c`, ejecuta `gcc -Wall -Wextra -c -o cualquier_cosa.o cualquier_cosa.c`.

### Funciones de Make
- `wildcard`: Busca archivos que coincidan.
  `SRCS = $(wildcard *.c)` (Encuentra todos los .c).
- Substitución de sufijos:
  `OBJS = $(SRCS:.c=.o)` (Reemplaza .c por .o en todos los nombres).

## 📝 Instrucciones

1. Reutiliza los archivos de código en `src/` (esta vez incluyen un `utils.c`).
2. Crea un `src/Makefile` que defina:
   - Una variable `CC`.
   - Una variable `CFLAGS` con `-Wall -Wextra -std=c17`.
   - `SRCS` usando `wildcard` para detectar los tres `.c`.
   - `OBJS` usando substitución para calcular los `.o` necesarios.
   - El objetivo principal `calculadora`.
   - La regla de enlazado genérica usando `$@` y `$^`.
   - La regla de compilación por patrón `%.o: %.c` usando `$@` y `$<`.
   - Un objetivo `.PHONY: clean` que borre los `.o` y el binario.

## ✅ Criterios de Éxito
- Ejecutar `make` compila exitosamente todos los archivos sin importar sus nombres, apoyándose en el `wildcard` y en las reglas de patrón.
- Los tests verifican el uso de las variables especiales.
