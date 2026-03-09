# El Signo Numeral indica Comentarios en GNU Makefile Sintaxis.
# ==========================================================

# 1. MACROS / VARIABLES DINÁMICAS (Para cambiar de compilador fasil Ej clang en Mac C++)
CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -O2

# Nombre de Tu Binario Final Desired Kernell OS
BINARIO_FINAL = calculos_make

# 2. EL TARGET MAGNO (ENTRY POINT C) 
# Make siempre ejecuta la meta "all" si no pides explicitamente una.
all: $(BINARIO_FINAL)

# 3. ENSAMBLAJE / LINKED FINAL BINARIO C 
# Depende de los sub-partes "main.o" y "mates.o". Make los evaluará recursivamente abajo.
# El $@ es una meta_variable magica C make que significa "EL NOMBRE DE ESTA META IZQUIERDA" (calculos_make).  
# El $^ es la meta var de "TODOS LOS REQUISITOS LISTADOS EN LA DERECHA" (main.o mates.o).
# ¡ASÍ SE EVITA REPETIR PALABRAS!
$(BINARIO_FINAL): main.o mates.o
	$(CC) $(CFLAGS) -o $@ $^

# 4. FORJA DE SUB-OBJETOS INTERMEDIOS (Compilación Separada! ".c" -> ".o")
# Flag '-c' = Compilame el Archivo, PERO NO ARRAUES el binario final todavia. Solo damelo a medias Maquina_Code!.
main.o: src/main.c
	$(CC) $(CFLAGS) -c src/main.c -o main.o

mates.o: src/mates.c
	$(CC) $(CFLAGS) -c src/mates.c -o mates.o

# 5. RECOLECTOR DE BASURAS
# `.PHONY` es crucial GnuMake: Le indica a Bash que "clean" no es un nombre de un Archivo Físico C 
# que Make crea, sino un comando Virtual, salvandote de que se bugee si justo TÚ tenias una carpeta llamada `clean` ahi mismo!.
.PHONY: all clean

clean:
	rm -f *.o $(BINARIO_FINAL)
	@echo "Limpieza GNU Asíncrona Terminada C."
