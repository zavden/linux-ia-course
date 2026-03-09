# 🚀 Proyecto Preparatorio — Auto-Compiler Dockerizado

## 🎯 Objetivo
Juntar las cuatro disciplinas: Básicos de comandos Linux, Bash Scripting, Mínimo de Makefile y Docker Containers efímeros.

Si logras implementar este script con éxito, estarás 100% listo para enfrentar el **Bloque 00** del curso oficial de `Linux-C`.

## 📋 Requisitos Funcionales

Tienes que escribir un script Maestro: `src/run_safe.sh` que haga lo siguiente:

1. **Recibir Argumento:** Tomará el path hacia un archivo `.c` local.
   Ej: `./src/run_safe.sh mimain.c`
2. **Validar:** Comprobar si le pasaste el fichero, y comprobar con `[ -f "$1" ]` si el fichero existe en verdad.
3. **Generar el Entorno bajo demanda:**
   - Debe usar `cat > Makefile << 'EOF'` para autogenerar un Makefile simple que compile tu `.c` genérico por un nombre fijo como `app.exe`.
   - Debe usar `cat` y `EOF` para autogenerar un `Dockerfile` a partir de `fedora:latest`, instalando GCC y dependencias. Al final del Dockerfile se debe compilar y lanzar el MAKE local montado.
4. **El truco de Docker Build:** Hacer `docker build` de ese Dockerfile efímero local silenciosamente (`>/dev/null`). De ponerle un Tag temporal como `-t comp-temp`.
5. **Ejecutar Efímero (Docker Run):** Hacer `docker run --rm -v "$PWD":/app -w /app comp-temp make all`.
6. Si compila (exit code 0), avisar: "✅ Compilado con exito!"
7. Si falla, avisar del error claramente, que compile un log, limpie el Dockerfile residual temporal, y se cierre exitosamente terminando. (Puedes agregar validación extra para ejecutar la app directamente si lograste armarlo).

## 💡 ¡Nota Importante!
El objetivo es que resuelvas un flujo de CI/CD simplificado donde metes un código y automáticamente un Docker aislado lo compila para ver si la sintaxis funcionaría en Fedora, dejando tu sistema host intacto.
