# Makefile Maestro de Proyecto Final Bloque 03 (minishell)
CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -O2

TARGETS = minishell

all: | build $(TARGETS)

build:
	mkdir -p build

minishell: src/minishell.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(TARGETS) test_out out.txt

.PHONY: all clean
