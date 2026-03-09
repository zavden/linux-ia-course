# 🎓 Precurso: Fundamentos de Linux, Bash, Docker y Make

> **Objetivo:** Nivelar conocimientos fundamentales necesarios antes de iniciar el **Bloque 00** del Curso Linux-C.
> **Orientación:** 100% Práctico ("Aprender haciendo").
> **Entorno:** Tu propia terminal Linux (Fedora/Debian/Ubuntu).

---

## 📋 Tabla de Contenidos

1. [Módulo A — Linux Core y Bash Scripting](#módulo-a--linux-core-y-bash-scripting)
2. [Módulo B — Git y Control de Versiones](#módulo-b--git-y-control-de-versiones)
3. [Módulo C — Compilación Manual en C y Makefiles](#módulo-c--compilación-manual-en-c-y-makefiles)
4. [Módulo D — Docker Desde Cero](#módulo-d--docker-desde-cero)
5. [Proyecto Final del Precurso](#proyecto-final-del-precurso)

---

## 🏗️ Estructura del Precurso

```
precurso/
├── MAIN.md                    # Este documento
├── mod_A/                     # Bash y CLI
│   ├── ej01_rutas/
│   ├── ej02_permisos/
│   └── ej03_scripting/
├── mod_B/                     # Git
│   └── ej01_repos/
├── mod_C/                     # C y Make
│   ├── ej01_gcc/
│   └── ej02_make/
├── mod_D/                     # Docker
│   ├── ej01_basico/
│   └── ej02_volumenes/
└── proyecto_pre/              # Proyecto final preparatorio
```

---

## 🐧 Módulo A — Linux Core y Bash Scripting

> **Objetivo:** Perder el miedo a la terminal. Entender permisos, redirecciones y automatización básica.

### Ejercicio A.1 — Navegación y Archivos
- **Conceptos:** `pwd`, `cd`, `ls -la`, `mkdir -p`, `touch`, `rm -rf`, `cp -r`, `mv`.
- **Tarea:** Crear una estructura de directorios anidada usando un solo comando. Mover archivos usando comodines (`*.txt`).
- **Prueba:** Script que verifica si la estructura quedó exactamente como se pidió.

### Ejercicio A.2 — Permisos y Usuarios
- **Conceptos:** `chmod`, `chown`, `su`, `sudo`, `id`.
- **Tarea:** Crear un script que solo pueda ser ejecutado por el usuario dueño (700) y un archivo de configuración de solo lectura (400).

### Ejercicio A.3 — Pipes y Redirecciones
- **Conceptos:** `>`, `>>`, `<`, `2>`, `|`, `grep`, `awk`, `cut`.
- **Tarea:** Procesar un archivo log de mentira. Extraer las líneas de "ERROR", contar cuántas hay, y guardarlas en `errores.txt`.

### Ejercicio A.4 — Bash Scripting Básico
- **Conceptos:** Variables, if/else, loops (`for`, `while`), argumentos (`$1`, `$@`), Exit Codes (`$?`).
- **Tarea:** Escribir un script `backup.sh <directorio>` que comprima el directorio en un `.tar.gz` verificando que el directorio exista primero.

---

## 🐙 Módulo B — Git y Control de Versiones

> **Objetivo:** Adquirir los hábitos necesarios para trackear los ejercicios del curso sin destruir el historial.

### Ejercicio B.1 — Trackeando un Proyecto
- **Conceptos:** `git init`, `git add`, `git commit -m`, `.gitignore`.
- **Tarea:** Repositorio en local. Crear commits simulando el avance de un ejercicio. Crear un `.gitignore` para omitir binarios (`*.o`, `build/`).

### Ejercicio B.2 — Máquina del Tiempo
- **Conceptos:** `git log`, `git status`, `git checkout` (o `git restore`), `git diff`.
- **Tarea:** "Romper" un archivo de texto intencionalmente y restaurarlo a la versión del último commit usando comandos de git.

---

## ⚙️ Módulo C — Compilación Manual en C y Makefiles

> **Objetivo:** Entender qué hace el IDE o CMake por debajo. Compilar a mano para valorar Make.

### Ejercicio C.1 — Del `.c` al Binario con GCC
- **Conceptos:** Compilador vs Linker. `gcc -c` vs `gcc -o`. Warnings (`-Wall`).
- **Tarea:** Compilar un programa de 3 archivos (`main.c`, `math.c`, `math.h`) manualmente paso a paso.
  1. `gcc -c math.c`
  2. `gcc -c main.c`
  3. `gcc -o calc main.o math.o`

### Ejercicio C.2 — Tu Primer Makefile
- **Conceptos:** Reglas, dependencias, variables simples (`$(CC)`). Exigencia de TABs.
- **Tarea:** Escribir un `Makefile` trivial (sin patrones avanzados) que automatice los tres pasos del Ejercicio C.1 y tenga una regla `clean`.

### Ejercicio C.3 — Makefiles de Nivel Intermedio
- **Conceptos:** Reglas de patrón (`%.o: %.c`), variables automáticas (`$@`, `$<`, `$^`), directiva `.PHONY`.
- **Tarea:** Refactorizar el Makefile de C.2 para que funcione con cualquier cantidad de archivos `.c` en el directorio.

---

## 🐳 Módulo D — Docker Desde Cero

> **Objetivo:** Entender los contenedores no como "máquinas virtuales mágicas", sino como procesos aislados.

### Ejercicio D.1 — El Mundo Efímero
- **Conceptos:** `docker run`, `docker ps`, imágenes vs contenedores.
- **Tarea:** Levantar un `ubuntu`, crear un archivo dentro, salir. Levantar otro `ubuntu`, mostrar que el archivo no está. Entender por qué.

### Ejercicio D.2 — Escribiendo un Dockerfile
- **Conceptos:** `FROM`, `RUN`, `CMD`, `COPY`, `WORKDIR`.
- **Tarea:** Crear una imagen basada en `alpine` que instale `curl`, copie un script de bash dentro, y lo ejecute por defecto.

### Ejercicio D.3 — Volúmenes (El Puente)
- **Conceptos:** `docker run -v host:container`.
- **Tarea:** Escribir un archivo fuente `hola.c` en tu PC. Levantar un contenedor gcc montando tu carpeta local. Compilar el archivo desde el contenedor pero que el `.exe` quede guardado en tu carpeta host.
  > *Este es el concepto más importante para el resto del curso.*

### Ejercicio D.4 — Docker Compose Básico
- **Conceptos:** Traducir comandos largos de `docker run` a un YAML mantenible.
- **Tarea:** Replicar el Ejercicio D.3 escribiendo un `docker-compose.yml`.

---

## 🏆 Proyecto Final del Precurso

### 🚀 "Auto-Compiler en Docker"

**Objetivo:** Combinar TODO. (Bash, Git, Make, Docker).

**Requisitos:**
1. Crear un script en Bash llamado `run_safe.sh`.
2. El script recibe como argumento un archivo `.c`.
3. El script automáticamente:
   - Crea un `Makefile` temporal.
   - Genera un `Dockerfile` al vuelo basado en `fedora`.
   - Lanza un contenedor Docker que monta el directorio actual.
   - El contenedor ejecuta `make` y luego ejecuta el binario resultante.
   - Si algo falla (Exit code != 0), el contenedor se destruye, pero deja el log de errores.
4. Todo el proyecto debe estar trackeado con `git`.

**¿Por qué este proyecto?**
Si logras hacer esto, el **Bloque 00** será un repaso, y estarás mentalmente listo para enfocarte 100% en el código en **C** durante todo el curso, en lugar de pelear con las herramientas del entorno.
