# Makefile Maestro Proyecto 04 Minitop C Proc Parseador
CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -O2

TARGETS = minitop

all: build $(TARGETS)

build:
	mkdir -p build

minitop: src/minitop.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(TARGETS) build test_out.log

.PHONY: all clean
