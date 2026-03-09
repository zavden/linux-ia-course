# Ejercicio D.3 — Volúmenes (El Puente Mágico)

## 🎯 Objetivo
Solucionar el problema del "Mundo Efímero" (D.1). Compartir código entre tu PC (Host) y el Contenedor sin tener que usar `COPY` ni reconstruir la imagen constantemente.

## 📚 Teoría Mínima
Los **Volúmenes** de montaje directo (Bind Mounts) vinculan un directorio de tu máquina anfitriona hacia un directorio *dentro* del contenedor.

Sintaxis:
`docker run -v /ruta/local:/ruta/contenedor mi-imagen`

A menudo se usa `$PWD` (Print Working Directory) para referirse al directorio actual:
`docker run -v "$PWD":/app mi-imagen`

¿Por qué es vital para este curso?
Porque así podrás escribir C en tu PC usando tu editor favorito, y compilarlo dentro de un contenedor Fedora/Debian instantáneamente sin tener que copiar el código fuente en cada cambio.

## 📝 Instrucciones

1. He dejado un simple `src/hola.c` ya preparado.
2. NO vas a escribir un Dockerfile. Usaremos directamente la imagen de `gcc:latest` que ya tiene compiladores instalados en Debian.
3. En la terminal, ubícate en la carpeta `ej03_volumenes` y ejecuta:
   ```bash
   docker run --rm -v "$PWD/src":/usr/src/miapp -w /usr/src/miapp gcc:latest gcc -o hola hola.c
   ```
   **Desglose del comando:**
   - `--rm`: Borrar el contenedor al terminar.
   - `-v "$PWD/src":/usr/src/miapp`: Monta tu carpeta `src` local dentro de la ruta `/usr/src/miapp` del contenedor.
   - `-w /usr/src/miapp`: Set Working Directory. Ejecuta los siguientes comandos desde esa carpeta.
   - `gcc:latest`: La imagen a usar.
   - `gcc -o hola hola.c`: El comando que se ejecuta.
4. Revisa tu carpeta local `src/`. ¡Debería haber aparecido mágicamente el archivo ejecutable `hola`!
5. Ejecútalo en tu máquina o pásalo por otro contenedor para probarlo.

## ✅ Criterios de Éxito
- Entiendes la magia del Bind Mount: el GCC corrió aislado en un Linux limpio, pero guardó el binario resultante de vuelta en tu disco duro físico a través del volumen compartido.
