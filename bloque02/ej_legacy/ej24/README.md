# Ejercicio 2.4 — Directorios e Inodos: `opendir`, `readdir`, `stat`

## 🎯 Objetivo
Aprender a iterar sobre los contenidos de un directorio usando la API del sistema de archivos POSIX. Entender qué es un inodo y cómo leer los metadatos de un archivo (tamáño, fecha, permisos) usando la estructura mágica `stat`.

## 📚 Teoría Mínima
- Un directorio en Linux no es más que un "archivo especial" que contiene una lista de nombres apuntando a números de Inodo.
- `opendir(ruta)`: Abre el "Directorio" devolviendo un `DIR *` (stream).
- `readdir(DIR *dir)`: Devuelve un puntero a un bloque `struct dirent` que contiene el nombre del archivo (`d_name`) y su tipo básico (`d_type`).
- `stat(ruta, &struct_stat)` o `lstat(...)`: Rellena una mega-estructura `stat` conteniendo absolutamente todo sobre un archivo: `st_size` (bytes), `st_uid` (dueño), `st_mode` (permisos y tipo), `st_mtime` (última modificación).
  - *Nota*: `lstat` es preferible al iterar para no "seguir" enlaces simbólicos infinitos.

## 📝 Instrucciones

1. En `src/main.c`, crea un programa tipo mini `ls -l`.
2. El programa acepta un argumento de directorio (ej: `./app /etc`).
3. Usa `opendir` para abrirlo. 
4. Itera con un bucle `while ((ent = readdir(dir)) != NULL)`.
5. Por cada elemento, aísla su nombre, ignora `.` y `..` si lo deseas, y constrúyelo en una ruta completa (ej: `/etc` + `/` + `passwd`).
6. Pásale esa ruta completa a `stat()` o `lstat()`.
7. Imprime en pantalla una línea como hace `ls -l`: 
   `[Tamaño_en_bytes]  [Tipo: Archivo o Dir]  [Nombre_del_archivo]` 

## ✅ Criterios de Éxito
- Has creado un mini-ls. Al ejecutar `./app .`, lista el código fuente. Al ejecutar `./app /`, lista la raíz con información validable de metadatos sacados del Kernel en vivo.
