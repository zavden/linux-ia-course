# Ejercicio 0.4 — Valgrind y GDB en Docker

## 🎯 Objetivo

Aprender a depurar programas en C detectando fugas de memoria con **Valgrind** y analizando crashes con **GDB** dentro de un contenedor Docker.

## 📚 Teoría Mínima

### Valgrind

Valgrind instrumenta tu código para detectar uso de memoria no inicializada, accesos fuera de límites y fugas de memoria (memory leaks).

- **Ejecución básica:** `valgrind ./programa`
- **Análisis detallado de fugas:** `valgrind --leak-check=full --show-leak-kinds=all ./programa`
- **Requisito:** Compilar con información de depuración (`-g`) y sin optimizaciones excesivas (`-O0`). *Nota: Desactiva los sanitizers (ASan) si usas Valgrind, ya que interfieren entre sí.*

### GDB (GNU Debugger)

GDB permite pausar la ejecución de un programa, inspeccionar variables y ver el stack trace (la traza de llamadas) cuando ocurre un crash (como un Segmentation Fault).

```bash
gdb ./programa
(gdb) run           # Ejecuta el programa
(gdb) bt            # Backtrace: ver dónde ocurrió el crash
(gdb) print var     # Imprimir el valor de una variable
(gdb) quit          # Salir
```

### El Problema de `SYS_PTRACE` en Docker

Por motivos de seguridad, los contenedores Docker por defecto bloquean la llamada al sistema `ptrace`, la cual es utilizada por GDB para "engancharse" al proceso.

Para usar GDB dentro de Docker, debes arrancar el contenedor con permisos adicionales:
```bash
docker run --rm -it --cap-add=SYS_PTRACE ej04-fedora /bin/bash
```

## 📝 Instrucciones

1. **Analiza el código en `src/main.c`:**
   Este código contiene **dos errores intencionales**: un bloque de memoria que no se libera, y un intento de acceso a memoria inválida (Segmentation Fault) en ciertas condiciones que vas a comentar/descomentar.

2. **Detecta el memory leak con Valgrind:**
   - Compila el programa usando `make debug` (que añade banderas de debug, pero asegúrate en el `Makefile` de apagar el sanitizer temporariamente o usa `make all` e inyecta `-g` manual si el target debug de tu Makefile actual tiene sanitizer que choca con Valgrind). Vamos a asumir `make all` con `CFLAGS="-g -O0"`.
   - Ejecuta valgrind dentro del contenedor. Observa el bloque reportado como "definitely lost".
   - Arregla el código en `src/main.c` (añade el `free()`).

3. **Analiza el crash con GDB:**
   - Descomenta la línea en `src/main.c` marcada con `// Descomentar para causar SegFault`.
   - Compila de nuevo.
   - Ejecuta el contenedor con `--cap-add=SYS_PTRACE`.
   - Usa `gdb ./build/main`, lanza `run`, espera el crash y usa `bt` para ver qué línea lo causó. Arréglalo.

4. **Tests:**
   - El test incluido automatizará ejecutar Valgrind detectando si tienes memory leaks.

## ✅ Criterios de Éxito

- [ ] Tras tus arreglos, `make run` ejecuta sin Segmentation Fault y produce la salida correcta.
- [ ] Ejecutar `valgrind --leak-check=full ./build/main` reporta "All heap blocks were freed -- no leaks are possible".
- [ ] El script `make test` pasa exitosamente.

## 📖 Referencias

- `man valgrind`
- `man gdb`
