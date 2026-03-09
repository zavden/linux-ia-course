# Ejercicio C.1 — Del `.c` al Binario con GCC

## 🎯 Objetivo
Entender el proceso de compilación en C en dos fases claras: Compilación (creación de objetos `.o`) y Enlazado (Linkeo para crear el binario final).

## 📚 Teoría Mínima
Los IDEs actuales ocultan el hecho de que compilar C no es un proceso de "un solo clic".

**Fase 1: Pre-procesado y Compilación**
Convierte el código en lenguaje de máquina intermedio (Object files). Si tienes un error de sintaxis, falla aquí.
```bash
gcc -c archivo.c
```
Esto genera un archivo `archivo.o`. Tienes que hacerlo para *cada* archivo `.c` de tu proyecto. El flag `-c` significa "Compile y Ensambla, pero No Linkees".

**Fase 2: Enlazado (Linking)**
Junta todos los archivos `.o` y librerías externas en un único archivo ejecutable. Si llamaste a una función que no existe, falla aquí ("undefined reference").
```bash
gcc -o programa archivo1.o archivo2.o
```

**Beneficio de la Separación:**
Si tienes 100 archivos `.c` y solo modificas `math.c`, no necesitas recompilar los otros 99. Solo recompilas `math.c` → `math.o` y vuelves a enlazar todo.

## 📝 Instrucciones

He dejado 3 archivos simples de código en `src/`:
- `math.h` y `math.c`: Una biblioteca matemática de juguete con una función `sumar`.
- `main.c`: El punto de entrada que usa `sumar`.

Abre tu terminal en la carpeta `src/` y hazlo manualmente comando a comando (no uses script, teclealo):

1. Compila `math.c`:
   `gcc -Wall -c math.c`
2. Compila `main.c`:
   `gcc -Wall -c main.c`
3. A este punto deberías ver `math.o` y `main.o` usando el comando `ls`.
4. Enlaza y genera el ejecutable `calculadora`:
   `gcc -o calculadora main.o math.o`
5. Ejecuta el programa:
   `./calculadora`

## ✅ Criterios de Éxito
- Has logrado crear y ejecutar `./calculadora` sin usar ningún sistema de build automatizado.
- Has entendido por qué se usa `-c` y por qué se omite al final.
