# Ejercicio A.2 — Permisos

## 🎯 Objetivo
Comprender el modelo básico de permisos en Linux (rwx).

## 📚 Teoría Mínima
Todo archivo tiene permisos para **U**suario dueño, **G**rupo y **O**tros (U-G-O).
Los permisos son **R**ead (4), **W**rite (2), e**X**ecute (1).
- `rwx` = 4+2+1 = 7.
- `rw-` = 4+2 = 6.
- `r--` = 4 = 4.

Comandos:
- `chmod 700 archivo` (User=rwx, Group=---, Other=---).
- `chmod +x script.sh` (Hace ejecutable para todos).
- `ls -l` te muestra `drwxr-xr-x` donde `d` es directorio y luego bloques de 3.

## 📝 Instrucciones

1. En `src/`, crea dos archivos vacíos usando tu terminal: `secreto.sh` y `config.cfg`.
2. A `secreto.sh`, asígnale permisos para que **solo el dueño** pueda leer, escribir y ejecutar (700).
3. A `config.cfg`, asígnale permisos para que **solo el dueño** pueda leer (nadie debe poder escribir ni ejecutar) (400).
4. Verifica los permisos con `ls -l`.

Para validar este ejercicio, el script de tests revisará directamente los permisos de tus archivos generados en `src/`.

## ✅ Criterios de Éxito
- `src/secreto.sh` tiene permisos `-rwx------`
- `src/config.cfg` tiene permisos `-r--------`
