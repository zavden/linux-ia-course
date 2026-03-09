# 📖 THEORY.md — Bloque 03: Procesos y Señales

El Bloque 03 marca la transición de un simple programador de C a un verdadero "Programador de Sistemas". Aquí entenderemos cómo el Kernel de Linux percibe tu programa, cómo lo duplica, de dónde vienen las famosas tuberías (`|`) de bash y la mecánica tras el mítico "Control+C".

---

## 1. El Nacimiento: `fork` y el PID

Todo programa en ejecución es un **Proceso**, identificado unívocamente por su PID (Process ID). Pero los procesos en Linux **no se crean de la nada**. Nacen duplicando a un padre. 

La syscall `fork()` es legendaria en sistemas UNIX. Cuando llamas a `fork()`:
1. Tu programa detiene su ejecución 1 milisegundo.
2. El Kernel fotocopia **EXACTAMENTE TODO** (RAM, código, memoria, cursores de archivos) creando un proceso Hijo (Child).
3. En el siguiente ciclo de CPU, **AMBOS** (padre e hijo) continúan ejecutando la siguiente línea de código tras el `fork()`. No saltan mágicamente; simplemente ahora hay dos clones en la misma línea.

**¿Cómo saben quién es quién?** Por el valor de retorno que `fork()` inyectó mágicamente a cada uno:
- Al Padre, `fork()` le devuelve el PID del hijo (ej. `1234`).
- Al Hijo, `fork()` le devuelve exactamente `0`.
- Si falló (PC sin memoria o límites excedidos), devuelve `-1`.

```c
pid_t pid = fork();
if (pid == 0)      { printf("Soy el hijo ejecutando el mismo codigo.\n"); }
else if (pid > 0)  { printf("Soy el padre, mi hijo es PID: %d\n", pid); }
```

---

## 2. Zombies (🧟‍♂️) y la Espera (`waitpid`)

Un axioma de Linux es que **"Los padres siempre deben sepultar a sus hijos"**. Y lo hacen con `wait()` o `waitpid()`.

- Si un hijo hace `exit(0)` y muere antes que el padre, sus recursos en RAM se borran casi todos, pero su registro (DNI) sigue "muerto en la tabla del Kernel". A este estado se le llama **Proceso Zombie** (`[Z]` o `defunct`). Es un cadáver con el único propósito de guardarle su Exit Status al padre.
- Un Zombie desaparece definitivamente de la tabla del sistema el instante exacto en el que el padre finalmente llama a `waitpid(&status)`, logrando leer ese exit code y "sepultándolo".
- Si el padre muere antes y jamás le hizo `wait()` a su zombie... el Sistema asume custodia (`init` en la antigüedad, SystemD / PID 1 en lo moderno) y lo asimila y entierra. A este proceso temporalmente abandonado por la muerte de su progenitor se le llama **Proceso Huérfano**.

---

## 3. Posesión y Destrucción Cerebral: La Familia `exec`

Clonar procesos sería inútil si los hijos siempre ejecutaran exactamente la misma copia fotocopiada de tu código por la eternidad. ¿Cómo logras que tu programa ejecute herramientas ajenas como `ls` o `tar`?

Primero clonas (con `fork`), y luego tú, o el hijo, deciden suicidiar la copia de su memoria local para cargar en su lugar el código de un disco duro externo. Para eso la familia mágica de funciones **`exec`** (execvp, execl, execle, etc).
Funciona como una posesión demoníaca: la syscall va al disco duro de tu linux, encuentra el binario que le pides y aplasta completa la RAM del proceso llamador matando irremediablemente la ejecución anterior. Ocurre reemplazo total de código pero mantienes los File Descriptors (archivos abiertos e identificadores) que tenías antes del aplastamiento. Si el `exec` tiene éxito... la función "nunca retorna" a ti, porque tu programa original C "ya no existe ahí".

Un Terminal (`bash`) clásico ejecuta comandos literal así:
- Hace `fork()`.
- El hijo entra al `if`. Hace un `exec("ls")` y se transforma en el comando "ls".
- El bash padre entra al `else` haciendo `waitpid()` esperando paciente a que ese "ls" termine y tire un exit status. 

---

## 4. Tuberías (Pipes) y Redirección de Archivos (`|`, `>`, `<`)

¿Cómo sabe `ls | grep` qué información enviarle al otro si no usan un txt?
La respuesta es usar **Pipes anónimos**. Con `pipe(int fds[2])`, logramos que el OS nos entregue dos File Descriptors interconectados internamente, el `fds[0] (boca de lectura)` y `fds[1] (boca de escritura)`. Si escupes bytes en el `fds[1]`, en lugar de llover al disco rigido, irán directamente a la cola que desemboca en `fds[0]`, sin salir jamás de memoria.

¿Pero cómo lo enviamos al comando `grep` sin que él lo sepa?
Nos aprovechamos de que todos asumen que `0` es teclado/stdin y `1` es pantalla/stdout. Y usamos la syscall **`dup2()`** para mentirles. El papá bash clona y reemplaza el ID `1` de "Pantalla" del hijo por el ID que nos dio la tubería mágica de escritura... Así que cuando el hijo inocentemente haga un mísero `printf()`, volcará silenciosamente los bytes a ese buffer ram y en vez de brillar los pixeles de tu monitor, la pipe recibirá el flujo directo hasta la boca de lectura.

---

## 5. Señales: El control inter-procesos agresivo (`Control+C` vs `SIGKILL`)

Las **Señales** (*Signals*) son interrupciones de software mandadas asincrónicamente por el sistema o por ti mismo.

- Cuando un usuario aplasta `Ctrl+C`, el kernel tira un misil balístico a tu proceso llamado `SIGINT`. Automáticamente, tú explotas a menos que antes de que ocurriese esto le avises formalmente al Kernel: *"Ignora (SIG_IGN) esto y mejor ejecuta esta función mágica"* registrando un **Handler**.
- Esto lo registramos modernamente usando **`sigaction`** (la API que reemplazó a `signal()`).
- Exite una señal mítica: **`SIGKILL` (Señal #9)**. Esta señal es indestructible. No la puedes "Registrar" y no la puedes "Ignorar". Es un comando perentorio del Kernel que aplasta y borra de memoria tu ejecución sin siquiera decirte chao. Jamás la usen al destruir sistemas a menos de emergencias de pánico crítico al no dar espacio a bases de datos de hacer sync o flushes a disco.

---

## 6. Monstruos asincrónicos y Demonios (Daemons)

Los Demonios son programas Linux del fondo (background), carecen de una terminal TTY asociada, son desvinculados por completo del humano. No leen Stdin y no escupen en Stdout porque no tienen pantallas a quién contarle... si hay problemas tienen que volcárselo a sistemas syslog a ciegas. Tienen que aislarse fuertemente abandonando los PIDs de bash (`setsid`) y cerrando todo I/O heredado. Un demonio moderno sin embargo es domado por `SystemD` lo cual trivializa gran cantidad de trabajo para implementarlo en simple C (Ej: Te perdona hacer el doble fork).
