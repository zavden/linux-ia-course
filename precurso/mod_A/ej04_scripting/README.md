# Ejercicio A.4 — Bash Scripting Básico

## 🎯 Objetivo
Aprender la sintaxis básica de los scripts de Bash: validación de argumentos, condicionales `if`, uso de variables y command substitution (`$(cmd)`).

## 📚 Teoría Mínima
- `#!/bin/bash` : Shebang. Le dice al SO qué intérprete usar.
- `$1`, `$2` : Argumentos posicionales (el primero, el segundo).
- `$#` : Número total de argumentos.
- `if [ condición ]; then ... else ... fi` : Condicionales. No olvides los espacios dentro de `[ ]`.
- `[ -d "$1" ]` : Verdadero si `$1` es un directorio y existe.
- `[ -z "$1" ]` : Verdadero si la cadena `$1` está vacía (zero length).
- `tar -czvf archivo.tar.gz carpeta/` : Comprimir. `-c` (create), `-z` (gzip), `-v` (verbose), `-f` (file).
- `date +%Y%m%d_%H%M%S` : Imprime la fecha actual en formato útil para nombres de archivos (Ej: `20260307_143000`).
- `$(comando)` : Command substitution. Captura la salida de un comando en una variable. Ej: `HOY=$(date +%Y)`

## 📝 Instrucciones

Crea un script `src/backup.sh` que haga lo siguiente:
1. Reciba el nombre de un directorio como primer argumento (`$1`).
2. Valide que se ha pasado al menos un argumento. Si no ( `$# -eq 0` ), imprima "Uso: ./backup.sh <directorio>" y salga con error (`exit 1`).
3. Valide que el directorio pasado realmente existe y es un directorio (`-d`). Si no, imprima "Error: El directorio no existe" y salga con error (`exit 1`).
4. Si todo es correcto, genere un nombre de archivo así: `backup_OMBREDELDIRECTORIO_FECHA.tar.gz`, donde FECHA es `$(date +%Y%m%d_%H%M%S)`.
   *Pista: Para extraer solo el nombre de la carpeta (sin la ruta entera), puedes usar `$(basename "$1")`.*
5. Ejecute el comando `tar` para comprimir el directorio apuntando al nuevo archivo `.tar.gz`.
6. Imprima "Backup completado: <nombre_del_archivo>".

## ✅ Criterios de Éxito
- Al llamar `src/backup.sh` sin argumentos falla limpiamente con el mensaje de Uso.
- Al llamar con un directorio inválido falla limpiamente.
- Al llamar con un directorio real (ej: `src/backup.sh /etc`), genera un archivo `backup_etc_2026....tar.gz`.
