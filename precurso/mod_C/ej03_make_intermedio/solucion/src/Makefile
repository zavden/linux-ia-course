CC = gcc
CFLAGS = -Wall -Wextra -std=c17
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

calculadora: $(OBJS)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f *.o calculadora
