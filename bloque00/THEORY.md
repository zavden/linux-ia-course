# 📖 THEORY.md — Bloque 00: Entorno y Docker

Bienvenido a la sección teórica del Bloque 00. Aquí estableceremos las bases necesarias para sobrevivir al curso de Desarrollo C en entornos Linux. El objetivo de este bloque es que domines **Make**, el compilador **GCC**, y el uso de **Docker** como entorno de laboratorio aislado, junto al entendimiento primordial de la memoria de C vía **Valgrind**.

---

## 1. El Proceso de Compilación en C (GCC)

El lenguaje C es un lenguaje compilado estáticamente. A diferencia de Python o Bash (que son interpretados línea por línea), C requiere que tu código sea traducido a lenguaje máquina (binario) antes de poder ejecutarse.

El proceso consta de 4 fases que GCC ejecuta encadenadas:
1. **Preprocesador (`gcc -E`):** Resuelve las líneas que empiezan con `#`. Pega el contenido de los `.h` adentro de los `.c`, expande Macros (`#define`) y quita los comentarios. El código sigue siendo texto.
2. **Compilador (`gcc -S`):** Toma ese texto gigantesco y lo traduce a Assembly (Ensamblador) específico para tu arquitectura (x86_64, ARM, etc).
3. **Ensamblador (`gcc -c`):** Traduce el Assembly a Código Objeto (archivos `.o`). Esto ya es código binario, pero está "incompleto": no sabe dónde están funciones que tú no escribiste, como `printf()`.
4. **Enlazador / Linker (`gcc -o` o `ld`):** Toma varios archivos `.o` y las librerías del sistema operativo (libc) y los "enlaza" armando el binario final ejecutable (`a.out` o lo que especifiques con `-o`).

**Flags esenciales de GCC para profesionales:**
- `-Wall -Wextra`: Activa todos los avisos comunes (Warnings). Son tu red de seguridad.
- `-Werror`: Convierte los Warnings en Errores fatales. Te obliga a escribir código perfecto.
- `-pedantic`: Obliga a cumplir el estándar ISO C estricto, impidiendo features que solo GNU o Clang permiten.
- `-std=c17`: Fija la versión del lenguaje a C17.
- `-g`: Inserta "Símbolos de Debug" en tu binario, permitiendo que GDB y Valgrind te digan exactamente el nombre del archivo y la línea en la que ocurrió un error (en vez de darte solo una dirección de memoria en hexadecimal).

---

## 2. GNU Make (Automatización)

Si tienes 50 archivos `.c`, no escribirás 50 llamadas a GCC. `make` es un motor de ejecución de tareas basado en dependencias.

Un archivo `Makefile` tiene "Reglas". Una regla tiene esta anatomía:
```makefile
objetivo: dependencia1 dependencia2
	comando (¡Debe estar indentado con TABULADOR, no espacios!)
```

Un Makefile profesional aprovecha:
- **Variables:** `CC=gcc`, `CFLAGS=-Wall -g`.
- **Variables Automáticas:**
  - `$@`: Expande al nombre del *objetivo*.
  - `$^`: Expande a *todas* las dependencias.
  - `$<`: Expande a la *primera* dependencia.
- **Reglas de Patrón (`%.o: %.c`):** Le dice a Make: "Para fabricar cualquier archivo que termine en `.o`, busca su respectivo `.c` y ejecuta este comando".
- **Objetos Falsos (`.PHONY`):** Si creas una regla llamada `clean` que borra archivos, y alguien por accidente crea un archivo que se llama "clean", `make clean` no hará nada porque creerá que el objetivo ya existe. `.PHONY: clean` arregla esto diciéndole a Make que `clean` es un verbo (una acción), no un archivo.

---

## 3. C y la Memoria: Stack vs Heap

La memoria RAM de tu programa en Linux se divide fundamentalmente en dos regiones.

### El Stack (La Pila)
- Es donde nacen las variables locales de tus funciones: `int x = 5;` o `char buffer[1024];`.
- Es memoria increíblemente estática, estructurada en LIFO (Last In First Out), y ultra rápida.
- El compilador sabe cuánto medirá desde antes abrir el programa, y **se borra/libera de RAM automáticamente** tan pronto como la función que las creó retorna su `return`.
- **Límite:** El Stack suele estar restringido a 8 MB por el sistema operativo (`ulimit -s`). Si intentas meter un arreglo de 10 MB aquí, tendrás un `Stack Overflow`.

### El Heap (El Montículo)
- Es memoria de tamaño libre, caótica, y dinámica. Se usa cuando no sabes cuánto va a ocupar algo hasta que el programa ya está corriendo (ej. cuánto mide un archivo que el usuario le pasó).
- Lo pides manualmente con la función `malloc(1024)`. 
- **Límite:** Todo lo que da la RAM de tu PC.
- La memoria no se limpia sola. Si sales de la función y olvidas soltar la RAM devolviéndosela al Kernel con `free(puntero)`, la RAM quedará ocupada pero tu programa ya habrá perdido cómo llegar a ella. A esto se le llama **Fuga de Memoria (Memory Leak)**. Eventualmente la RAM de la PC se acabará y morirá.

### Validaciones con VALGRIND
`valgrind --leak-check=full ./mi_programa` es una máquina virtual (un hipervisor minúsculo) que corre tu programa en cámara lenta (10-20x veces más lento). 
Interviene en cada llamada a CPU y se da cuenta si hiciste `malloc()` pero te fuiste de tu programa si llamar un `free()` del mismo tamaño o si pasaste más allá del límite de las variables e invadiste memoria ajena (Inválid Reads).

---

## 4. Docker: Contenedores para Experimentación

Instalar librerías crudas, editar parámetros de Kernel y borrar directorios raíz (`/`) como parte de tu aprendizaje destrozará al fin y al cabo tu instalación real y de paso tu Linux matriz.

Para evitar esto, usamos Docker y `docker-compose`. 
Docker utiliza dos mecanismos nativos de Linux (sin virtualizar discos duros o CPU extras):
- **Namespaces:** Aísla la visión del contenedor. Cree que está solo, que es el único árbol de carpetas `/`, sus procesos tienen PIDs desde cero (es el número 1).
- **Cgroups:** Le restringen cuánta RAM y CPU puede coger.

Creamos una receta por texto `Dockerfile`:
```Dockerfile
FROM debian:latest
RUN apt install gcc make -y
```
Y luego un plano (`docker-compose.yml`) que levanta y mapea un **"Bind Mount"**. Un bind mount ata instantáneamente una de tus carpetas locales (tu host) dentro de una carpeta local del contenedor (ej. un `volume: - ./:/workspace`). Así tu host edita el archivo C en tu VSCode, pero se compila por magia dentro de un Debian puro de terminal.

---

## 5. El entorno: bash y Variables de Contexto (Environment)
Los procesos Linux reciben datos del sistema mediante "Environment Variables".
Tu programa C en realidad nace en `getenv("VARIABLE")`. Esto es mucho más cómodo en sistemas contenerizados (DevOps y Docker) que tener que hacer un `fopen` y leer un tedioso `.ini` de texto.

**Errores a evitar:** JAMÁS alteres o modifiques el string que la función `getenv` te devuelve en memoria de C, esto corrompería instantáneamente el string subyacente que está alojado en un bloque crírtico (El Segmento BSS o DATA de la memoria global del Proceso en el OS). Si necesitas manipular un `getenv`, debes de copiarlo antes con `strcpy`.
