# Makefile Maestro de Proyecto 05 (miniserver Web Thread Pool)
CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -O2 -pthread

TARGETS = miniserver

all: build $(TARGETS)

build:
	mkdir -p build

miniserver: src/miniserver.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(TARGETS) build test_out.log

.PHONY: all clean
