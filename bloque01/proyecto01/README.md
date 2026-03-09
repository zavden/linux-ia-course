# 🚀 Proyecto 1 — Clones de Utilidades POSIX: `miniecho` y `minicat`

## 🎯 Objetivo
Hacer uso extenso del manejo de arrays, strings, parseo de argumentos CLI y la API de Archivos de C estándar (`stdio.h`) para recrear dos de las herramientas más icónicas de Unix: `echo` y `cat`.

## 📋 Requerimientos: `miniecho`

Debe comportarse exactamente igual al `echo` real de GNU Coreutils.
- Si no se le pasan argumentos, simplemente imprime un salto de línea (`\n`).
- Imprime todos los argumentos separados por un solo espacio.
- Opción `-n`: No imprime el salto de línea final.
- Opción `-e`: Activa la interpretación de las siguientes secuencias de escape:
  - `\n` : Nueva línea.
  - `\t` : Tabulador.
  - `\\` : Una contrabarra literal.
  - `\c` : Aborta inmediatamente la salida (no imprime nada más ni el salto de línea).

## 📋 Requerimientos: `minicat`

Debe comportarse como el `cat` real de GNU.
- Si se le pasa uno o más nombres de archivos, abre y concatena sus contenidos en consola secuencialmente.
- Si un archivo no existe, debe imprimir un error (usando `perror` o similar) y continuar con el siguiente archivo, saliendo al final con código de error.
- Si se le pasa un guion simple `-` como nombre de archivo (o si no se le pasan archivos), debe leer desde la Entrada Estándar (`stdin`) e ir repitiendo todo lo que el usuario tipea (hasta presionar Ctrl+D / EOF).
- **Extra opcional:** Opción `-n` (numera todas las líneas, empezando por 1) y `-b` (como `-n`, pero solo numera las líneas NO vacías).

## ✅ Criterios de Éxito
- Tus binarios compilados `./miniecho` y `./minicat` pasan todos los tests contra el output esperado oficial.
- El build-lab dual que hicimos en el Bloque 00 compilará esto en Fedora y Debian mágicamente.
- Asegúrate de emitiir `EXIT_FAILURE` si minicat no pudo abrir UN archivo de la lista, pero `EXIT_SUCCESS` si todo salió bien.
