# 📖 THEORY.md — Bloque 01: Fundamentos de C en Linux

En este bloque damos por sentado cómo compilar y estructurar proyectos, de modo que toda la atención se centra puramente en las entrañas del lenguaje `C` interactuando con las librerías POSIX por defecto.

---

## 1. El array de Argumentos: `argc` y `argv`

La firma principal en Linux C es: `int main(int argc, char *argv[])`
- `argc`: ("Argument Count"). Representa siempre al menos un número **1**.
- `argv`: ("Argument Vector"). Es una matriz de strings (punteros a punteros a char).
  - `argv[0]`: Siempre es el nombre con el que se invocó el programa (Ej: `./mi_app`). Jamás confíes en que estará limpio, esto depende totalmente de cómo el usuario y su shell decidieron llamarlo.
  - `argv[argc]`: El estándar de C garantiza que el array siempre terminará con un puntero nulo (`NULL`), lo que nos deja iterar usando `while(argv[i] != NULL)` sin temor a violar los límites.

### Parseo Complejo (`getopt`)
C no obliga al formato UNIX (-v, -a, --verbose). Es solo cultura.
Sin embargo, `getopt_long` es la utilidad estándar que nos salva la vida leyendo bandejas CLI:
- Maneja opciones que toman sub-valores: `-o miarchivo.txt`.
- Sabe combinar caracteres simples: `-vna` es lo mismo equivalente a `-v -n -a`.
- Expone tres utilidades mágicas: `optarg` (el puntero donde reposa tu argumento), `optind` (el índice actual sobre el que el cursor del array argv sigue saltando) y `opterr` (bandera para que no imprima basurilla de que algo falló y tú controles todo el mensaje).

---

## 2. Strings Seguros y C

"String" es una abstracción que no existe en C. Lo que sí hay, son "Arrays de caracteres finalizados en Null (NULL-TERMINATED Arrays)".

`char msg[5] = "Hola";`
Ocupa cinco bytes precisos. La letra 'H', 'o', 'l', 'a' y el letal `\0` al final. Funciones como `strlen` o `printf` en realidad avanzan un salto de memoria tras otro imprimiéndolos en tu consola eternamente, hasta que tropiezan con un byte vacío igual a `0`.

**El ataque de "Buffer Overflow" (Desbordamiento de búfer):**
Si llamas a `strcpy(buffer2, texto_ajeno_muy_grande)`, y buffer2 mide solo 5 bytes de límite pero el texto ajeno es de 50 bytes, `strcpy` no tiene ojos ni sabe cuánto mide tu arreglo de C, así que escribirá ciegamente todos los datos "cayéndose" del arreglo e invadiendo tus otras variables en memoria, pisoteándolas e inclusive, permitiendo inyecciones de código.

**La solución segura (`strlcpy` o snprintf):**
Oblígate a usar funciones donde uno de los parámetros es **SIEMPRE ES EL TAMAÑO MÁXIMO DEL RECIPIENTE**. (Ejemplo: `snprintf` o las variantes de OpenBSD strlcpy). Al pasárselos, aseguramos un truncado seguro (cutoff) que impide la desgracia si un usuario metio un path infinitamente gordo. 

---

## 3. Punteros en formato Genérico (`void *`) y Casteo

Un puntero es solo un número entero (`uintptr_t`) de 64 bits en tu Linux x86_64, que representa una ubicación física dentro de este inmenso bloque lineal imaginario que es tu memoria RAM de 16 GBs.
- `int *p_i`: Cuando sumas +1 (`p_i++`), saltarás 4 posiciones de bytes adelante porque él "sabe" que está atado a la estructura humana de enteros que miden el peso de 4 bytes.
- `char *p_c`: Cuando sumas +1, solo saltas 1 miserable byte adelante, porque C sabe que el "peso" del tipo humano de un character es un (1) byte unitario.

Un **`void *`** desata la anarquía. Es un puntero universal con el que no puedes hacer la aritmética anterior y que tampoco puedes desrreferenciar (leer sus valores directos con `*`) porque el compilador llora y levanta las manos al no saber de "qué peso" es lo que está mirando.
Se usa en la creación de contenedores de RAM masiva (`malloc`, bibliotecas de `linked_lists`) sirviendo de transporte de paquetería invisible. De modo que, si tu Linked List fue escrita recibiendo `void *`, después eres tú, programador, el que le promete explícitamente al compilador un "cast" de regreso: `(int *)`, sabiendo tú mejor que él, de que aquello originalmenre sí que pesaba un entero.

---

## 4. Retornos POSIX, "goto cleanup" y errno

Todo este bloque busca alejarte de C puro y meterte en la vertiente UNIX-Kernel estandarizada (conocido en libros como POSIX - *Portable Operating System Interface*).

En UNIX, todo falla. Un cable se desenchufa, un bloque se corrompe en el disco duro, le robamos los permisos a un archivo mientras estábamos a la mitad sacándole una query, etc.
Por ello, una llamada en el código del kernel (una Syscall), nunca lanza misteriosas "Excepciones" modernas de alto nivel, estas retornan el número `-1`.

¿Cómo sabes POR QUÉ dio -1? Usando **`errno`** (Número de Error). Una diminuta variable global Thread-Safe en la que el Kernel nos inyecta un cóodigo para saber el estado real de la interrupción. (2 significa "File not found", 13 significa "Permiso Denedago" [EACCES]).
No las leerás numéricamente; puedes invocar `perror("Tu frase")` para que mágicamente imprima la traducción al idioma natal desde la tabla del OS (e.j: "Tu frase: Permission denied").

**El patrón "Goto Cleanup":**
Tu profesor de informática seguramente exclamó "¡Prohíbido usar sentencias `goto` en C!" porque originaban lo llamado 'Código Espagueti', rebotando arriba y abajo.
En Ingeniería Operacional (inclusive las millones de líneas del Linux Source Code lo hacen), es una de las pocas ocasiones aprobadas internacionalmente para limpiar fugas de memoria y destruir cierres de recursos a la hora de presentarse catástrofes y abortar funciones en desastre, manteniendo una puerta limpia, unificada y elegante al final del esqueleto de cada función.
