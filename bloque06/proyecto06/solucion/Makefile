# Makefile Maestro de Proyecto 06 (minicache Epoll LRAM DB)
CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -O2

TARGETS = minicache

all: build $(TARGETS)

build:
	mkdir -p build

minicache: src/minicache.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(TARGETS) build test_out.log

.PHONY: all clean
