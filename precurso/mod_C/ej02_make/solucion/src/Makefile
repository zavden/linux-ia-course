# Makefile simple sin variables genéricas
calculadora: main.o math.o
	gcc -o calculadora main.o math.o

main.o: main.c math.h
	gcc -Wall -c main.c

math.o: math.c math.h
	gcc -Wall -c math.c

clean:
	rm -f *.o calculadora
