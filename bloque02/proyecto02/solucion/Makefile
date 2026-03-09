# Makefile Maestro para el Proyecto 02
CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -O2

# Objetivos finales binarios
TARGETS = minifind

all: $(TARGETS) | build

# Creacion del directorio oculto de build si hiciese falta a futuro o para acomodo
build:
	mkdir -p build

minifind: src/minifind.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS)

.PHONY: all clean
