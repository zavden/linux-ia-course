# Makefile Maestro para el Proyecto 01
CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -O2

# Objetivos finales binarios
TARGETS = miniecho minicat

all: $(TARGETS) | build

# Creacion del directorio oculto de build si hiciese falta a futuro o para acomodo
build:
	mkdir -p build

miniecho: src/miniecho.c
	$(CC) $(CFLAGS) -o $@ $^

minicat: src/minicat.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS)

.PHONY: all clean
