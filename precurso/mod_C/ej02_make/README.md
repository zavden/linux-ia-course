# Ejercicio C.2 — Tu Primer Makefile

## 🎯 Objetivo
Automatizar los pasos tediosos aprendidos en C.1 usando **GNU Make**.

## 📚 Teoría Mínima
`make` lee un archivo llamado `Makefile` que contiene Reglas.
Las reglas tienen este formato estricto:

```makefile
objetivo: dependencia1 dependencia2
<TAB>comando para construir el objetivo
```

> [!CAUTION]
> El sangrado debajo de "objetivo" **TIENE** que ser un caracter TAB REAL, no espacios. Si usas espacios, Make dará el error "missing separator".

### Reglas para un proyecto de 2 archivos
Para automatizar lo que hiciste en C.1 explícitamente, escribiríamos:

```makefile
# 1. El objetivo final depende de los archivos .o
calculadora: main.o math.o
	gcc -o calculadora main.o math.o

# 2. main.o depende de main.c (y sus headers)
main.o: main.c math.h
	gcc -Wall -c main.c

# 3. math.o depende de math.c
math.o: math.c math.h
	gcc -Wall -c math.c
```
Cuando ejecutas `make calculadora`, lee el archivo de arriba hacia abajo:
- Quiere construir `calculadora`. Pero nota que necesita `main.o` y `math.o`.
- Busca las reglas de cómo construir `main.o` y ejecuta el comando.
- Luego construye `math.o`.
- Finalmente enlaza construyendo la `calculadora`.

## 📝 Instrucciones

He dejado los mismos archivos `math.c`, `math.h` y `main.c` en `src/`.

1. Crea un archivo llamado literalmente `Makefile` (M mayúscula) dentro de `src/`.
2. Escribe a mano las reglas descritas arriba asegurándote de usar TAB para la indentación.
3. Añade además una regla especial de limpieza al final:
```makefile
clean:
	rm -f *.o calculadora
```
4. Abre la terminal en `src/` y prueba:
   - Ejecuta `make calculadora`. Verifica que se construyó todo.
   - Ejecuta `make clean`. Verifica que se borraron los generados.

## ✅ Criterios de Éxito
- El archivo `src/Makefile` funciona sin fallar por errores de sintaxis o de tabulación.
- `make clean` deja el directorio limpio de nuevo.
