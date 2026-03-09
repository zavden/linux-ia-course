# Makefile para compilar los ejemplos resueltos del Bloque 01

CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c17 -g

# Obtener todos los archivos .c
SRCS = $(wildcard *.c)
# Generar los nombres de los binarios
BINS = $(SRCS:.c=)

.PHONY: all clean

all: $(BINS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(BINS)
