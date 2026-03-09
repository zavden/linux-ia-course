# Ejercicio 2.1 — I/O de Bajo Nivel: open, read, write, close

## 🎯 Objetivo
Aprender a manejar archivos en Linux usando directamente las llamadas al sistema (syscalls) del kernel, sin usar la librería estándar de C (`stdio.h`). Comprender el concepto de *File Descriptor* (FD).

## 📚 Teoría Mínima
En UNIX, "Todo es un archivo". Cuando abres un archivo, el kernel te devuelve un **File Descriptor** (un simple número entero, como `3`, `4`, `5`...).
- `0` es Stdin (teclado).
- `1` es Stdout (pantalla).
- `2` es Stderr (errores).

**Syscalls clave (`<fcntl.h>`, `<unistd.h>`):**
- `int fd = open("archivo.txt", O_RDONLY);` (Lee)
- `int fd = open("nuevo.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);` (Escribe)
- `ssize_t bytes_leidos = read(fd, buffer, sizeof(buffer));`
- `ssize_t bytes_escritos = write(fd, buffer, bytes_leidos);`
- `close(fd);`

**¿Por qué usar syscalls y no `fopen()`?**
`fopen/fread` (stdio) usan "buffers mágicos" en el espacio de usuario para ser rápidos leyendo de a poquitos. Las syscalls `open/read` van directas al Kernel. Si lees de a 1 byte con syscalls, el programa será lentísimo. Si lees de a 4KB, será rapidísimo.

## 📝 Instrucciones

El objetivo es crear un programa iterativo de copia de archivos equivalente a `cp origen destino`.

1. En `src/main.c`, parsea dos argumentos: el archivo origen y el archivo destino.
2. Abre el origen en modo solo lectura (`O_RDONLY`).
3. Abre (o crea) el destino en modo escritura (`O_WRONLY | O_CREAT | O_TRUNC`), asegurando que, si se crea, tenga permisos `0644`.
4. Define un buffer de un tamaño específico, por ejemplo `4096` bytes (4KB, el tamaño óptimo típico de una página de memoria en Linux).
5. Usa un bucle `while` llamando a `read()`. Mientras devuelva `> 0` bytes, usa `write()` para vaciar el buffer en el archivo destino.
6. Maneja errores (revisa retornos de open, read, write) usando `perror()` y asegurándote de usar `close()` al final o en `goto cleanup`.
7. **Opcional/Avanzado:** Haz que el tamaño del buffer se pueda inyectar en compilación o línea de comandos para experimentar la diferencia de tiempo entre copiar con buffer de 1 Byte vs 4096 Bytes.

## ✅ Criterios de Éxito
- Tu `main.c` copia archivos binarios pesados (ej. `/bin/bash` o un mp4) de forma perfecta, creando un clon exacto (`md5sum origen == md5sum destino`).
- Cierras limpiamente los file descriptors al acabar.
- Manejas correctamente los errores de permisos (Ej. intentando escribir en `/etc/shadow`) devolviendo EXIT_FAILURE e imprimiendo la razón con `perror`.
