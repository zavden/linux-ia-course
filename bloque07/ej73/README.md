# Ejercicio 7.3 — Automatizando el Arte: Diseñando `Makefiles`

## 🎯 Objetivo
Dejar de escribir `gcc` a mano para siempre. Aprenderás a crear un Archivo GNU Maestro que sepa exactamente cómo compilar un proyecto enorme compuesto de 3 archivos C distintos, entrelazando Objetos (`.o`), Librerias Estáticas, y Banderas de compilacion Automáticas, reutilizando pasos en base a Fechas del Kernel OS.

## 📚 Teoría Mínima
- Nunca metas todo el codigo en 1 solo `main.c`. 
- Divídelo:
  - `mates.h`: Declaración de `int sumar(int a, int b);`. Los header NO se compilan. Sólo se importan con `#include "mates.h"`.
  - `mates.c`: El código real de las sumas y math C.
  - `main.c`: El principal que llama a la libreria Math.
- Si compilaras manual sería: `gcc -Wall -O2 main.c mates.c -o super_calculadora`
- En **Makefile**, declaras Metas (Targets).
- Una meta es la app final `super_app:` (Dos puntos). Abajo *debes* usar una Tablulación y pones la orden OS de esa fase.
- Puedes inyectar Variables: `CC = gcc` / `CFLAGS = -Wall -O2`. Luego usándolas en base a las mágicas macros Unix `$(CC) $(CFLAGS) ...`

## 📝 Instrucciones

1. Crea la estructura `src/mates.h`, `src/mates.c` y `src/main.c`. (Mates tendrá funciones de suma, resta y un Main tonto que las pruebe).
2. Construye el grandioso archivo a nivel de root y llámalo estrictamente **`Makefile`** (con la M mayúscula).
3. Dentro: 
   - Fija Variables: `CC = gcc`. `INCLUDES = -I./src`. `BANDERAS = -Wall -Wextra -O2`.
   - Crea el Target de objeto Mates: `mates.o: src/mates.c`
      - `\t $(CC) $(BANDERAS) -c src/mates.c -o mates.o`
   - Crea el Target de objeto Principal Main: `main.o: src/main.c`
      - `\t $(CC) $(BANDERAS) -c src/main.c -o main.o`
   - Crea el Target Final Compilador y Linkeador de binarios (El que junta los .o a maquina final EXE!): `mi_calculadora: mates.o main.o`
      - `\t $(CC) $(BANDERAS) mates.o main.o -o mi_calculadora`
   - Crea un meta `all:` que dependa de `mi_calculadora`. (All es lo 1ro que busca el comando Make al tipear en consola por defecto !).
   - Crea una meta genérica Limpiadora de Basura Kernell OS (Siempre se ocupa para subir y no saturar git) `clean:`:  `rm -f *.o mi_calculadora`

## ✅ Criterios de Éxito
- Te percatarás de que tú puedes tipear felizmente en tu bash el simple y diminuto comando: `make`.
- Magicamente el Toolchain lee, evalúa las flechas `.o`, corre TRES GCC's distintos perfectos y te arrojará el Binario C a la mano ahorrándote horas de CLI Tiping en proyectos inmensos.
