# 🚀 Proyecto 2 — `minifind`: Escáner Recursivo de Filesystem

## 🎯 Objetivo
Combinar lectura de directorios, metadatos (`stat`), y manejo avanzado de memoria/strings estructurando un programa en C sólido que imite al legendario comando `find` de UNIX.

## 📋 Requerimientos

Crea el clon `minifind`. 
Uso básico: `./minifind <directorio_base> [opciones]`

Las opciones esperadas son:
- `-name <str>`: Filtra que el nombre del archivo exacto sea `<str>`. (No requiero `fnmatch` u operadores globales `*` para este proyecto por simplicidad, una igualdad strcmp exacta es suficiente).
- `-type <c>`: Filtra por tipo de archivo. Soportar `d` (directorio), `f` (archivo regular) y `l` (symlink).
- `-size <num>c`: Filtra que el tamaño exacto en bytes sea el dado.

**Comportamiento fundamental:**
1. Si no hay path base, usar el directorio actual (`.`).
2. El escaneo **DEBE** ser recursivo (si encuentra un directorio, debe entrar y buscar ahí adentro). *Asegúrate de NO reentrar infinitamente ignorando el auto-link `.` y el link paterno `..`.*
3. Si un archivo matchea los filtros (o no hubieron filtros dados), debe imprimir su ruta completa en consola. (Ej: `./origen/docs/archivo.txt`).

## ✅ Criterios de Éxito
- Te enfrentarás y resolverás inteligentemente la combinación de paths recursivos en memoria de C (usando correctamente strings o alojando buffers dinámicos para los nombres concatenados).
- Tu programa logrará encontrar un único archivo escondido 4 niveles de profundidad abajo.
- Logrará diferenciar limpiamente un directorio de un archivo usando macros `S_ISDIR`.
