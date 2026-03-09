# Ejercicio 3.2 — Posesión Infernal: La Familia `exec` y `dup2`

## 🎯 Objetivo
Aprender cómo un proceso se auto-destruye para reencarnar cargando el código binario de otro programa en su RAM utilizando `exec`. También descubriremos cómo desviar el flujo de texto (Stdio/Stdout) de un proceso hacia archivos usando la syscall de manipulación de file descriptors `dup2()`.

## 📚 Teoría Mínima
- `execvp("ls", args)`: Busca "ls" en tu `$PATH`, destruye todo rastro de tu programa actual C, y desde esa línea en adelante, ¡tu proceso ahora es el comando "ls"! JAMÁS regresa un exit code a tu código, de hecho, si la función llega a ejecutar la línea de abajo de ella, significó que la posesión **falló** (porque si hubiese sido exitosa ya no existirías).
- **Los File Descriptors se heredan**. Si abriste un archivo (fd=3) y luego haces `exec("ls")`, el nuevo "ls" mantiene acceso al fd 3. 
- `dup2(oldfd, newfd)`: Desenchufa un cable de file descriptor y lo enchufa encima de otra ranura a la fuerza, pisando la actual si la había.

**El Santo Grial de la redirección de Bash (`ls > salida.txt`):**
1. Un padre (`bash`) hace `fork()`.
2. El Hijo abre (crea) el archivo `salida.txt` obteniendo un FD al azar (ej. `3`).
3. El Hijo desvía el cable mágico de monitor estándar: `dup2(3, STDOUT_FILENO)`.
4. El Hijo se auto-destruye cargando `execvp("ls", ...)`.
5. "ls" no sabe qué le hicieron, pero cuando él inocentemente le da `printf("archivo.c\n")`, en vez de salir en el monitor, `dup2` enruta el char al disco rígido `salida.txt`.
6. El Padre no sufre ningún corte eléctrico de monitor, el sigue con su Stdout bien sano conectado a PTY, esperando (`wait()`) pacientemente a que su desfigurado hijo termine.

## 📝 Instrucciones

Construye en `src/main.c` un programa que toma como argumento un comando Bash que el usuario te dé y un nombre de archivo (ej. `./app "ls -la" salida.txt`).
Debes orquestar que tu programa se burle del OS invocando el truco del desvío `>` para que el comando acabe volcando su contenido de texto al archivo en vez de la consola principal.

1. Has un `fork()`.
2. En el bloque del hijo:
   - Abre el archivo destino `salida.txt` con modo `WRONLY | CREAT | TRUNC`.
   - Fíja el tubo mágico Stdout hacia tu archivo con `dup2()`.
   - Llama a `execlp` o `execvp` invocando el comando bash real solicitado.
   - Si execvp falla, no olvides poner un `perror()` y un `exit(FAILURE)`.
3. En el bloque del padre:
   - Haz un clásico `waitpid` o `wait` hasta que el usurpador muera solo.
   - Deberás retornar o avisar al final *"El comando fue ejecutado e interceptado con éxito en su cárcel textual"*.

## ✅ Criterios de Éxito
- Has evitado llamar a la tentadora y fácil función externa `system()`, las reglas obligan a reescribir la lógica tu mismo orquestando el clonado, la canalización, y el lanzamiento binario.
