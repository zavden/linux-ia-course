# 🚀 Proyecto 03 — `minishell`: Construyendo tu propia Consola Bash

## 🎯 Objetivo
Unificar todo el Bloque 3: `fork`, `execvp`, `dup2`, `pipe` y `sigaction` para crear un intérprete de comandos interactivo (Shell) totalmente funcional, que te haga finalmente entender qué es y cómo funciona el propio entorno Bash de Linux que has estado usando toda la vida.

## 📋 Requerimientos Completos

### 1. Interfaz Básica
- Un bucle infinito mostrando un Prompt (`minishell:/home/davo$ `). Usa `getcwd()` para sacar la ruta.
- Leer texto crudo del usuario (ej: con `fgets` o `getline`).
- Parsear (dividir) la orden en un arreglo de strings (tokenizar por espacios como `argv`) aislados y terminados mágicamente en un NULL, para agradar a `execvp`.

### 2. Comandos Built-in (Mágicos Locales)
Los *built-ins* no pueden forkesearse (no corren externamente porque necesitan mutar o acceder a la RAM del padre vivo local). Si lo forkeas, un hijo cambia su directorio a `/root` y muere, pero tu Shell-Padre jamás se habría movido de `/`.
- Implementar `cd <dir>`. Llamando a la syscall local `chdir()`.
- Implementar `exit`. Abandona el while y muere el shell pacíficamente.
- Implementar `pwd`.
- Implementar `env` o `export` (puedes usar la variable C cruda `extern char **environ` para listar cosas).

### 3. Procesamiento Asíncrono (`&`)
- Si el usuario pone al final de la linea un ampersand `comando &`, el Shell **no** le hace un bloqueo `waitpid()` suspendiéndose. Le devuelve su línea del Prompt al vuelo inmediatamente.
- Registra un manejador para la señal `SIGCHLD`. Todo tu basurero de hijos desatendidos enviarán un balazo asíncrono pasivo a ti cuando mueran solos. Tu shell deberá detectarlo por detrás, recogiéndoles el Exit Code en el Handler con `waitpid(-1, NULL, WNOHANG)`. ¡Esto aniquilará a los zombies sin travar tu CLI!

### 4. Operación Tubería IPC (`|`)
- Soportar 1 nivel simple (mínimo) de pipeline (Ej: `ls -la | wc -l`).
- Divide en dos arreglos en la vida real. Haz 2 `forks()`. 
- El Hijo Izquierdo (`ls`) ata su `dup2` de STDOUT sobre la boquilla WRITE(1) del tubo. 
- El Hijo Derecho (`wc`) asfixia su `dup2` STDIN pegándolo en la boquilla READ(0). 
- Padre cierra ambas partes del tubo central globalmente, y les hace 2 `waitpid()`.

### 5. Control de Desastres (`SIGINT`)
- Un humilde `bash` JAMÁS muere si un usuario iracundo pulsa `Ctrl+C`. (Se imagina que te cierre las SSH de la universidad por error cada vez que canceles un script?).
- El Padre usará el registro `sigaction(SIGINT)` simplemente ignorándolo (usando `SIG_IGN`), o emitiendo un salto de línea vacía. Los *Hijos en ForeGround* locales que instancies heredarán pero ellos SÍ deben morir crasheados frente a este disparo de teclado de control.

## ✅ Criterios de Éxito
- Lanzas `./minishell`, haces `pwd`, te ubicas. Lanzas `ls -la | grep src` y te saca datos coloridos. Lanzas `sleep 5 &` y puedes seguir escribiendo y, a los 5 segundos detrás de las sombras de consola, te sale "[Jobs] PID 15535 exited".
