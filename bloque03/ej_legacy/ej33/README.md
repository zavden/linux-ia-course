# Ejercicio 3.3 — Inter-Process Communication (IPC): Pipes Anónimos

## 🎯 Objetivo
Entender el mecanismo fundamental de comunicación de datos en Linux sin usar archivos en disco rígido. Lograrás que el Padre y el Hijo se envíen mensajes de texto a través del Kernel usando Tuberías (Pipes) bidireccionales.

## 📚 Teoría Mínima
- Las Variables locales NO se comparten tras un `fork()`, cada uno tiene su cópia.
- Para hablar, Linux te regala un "Tubo" (`pipe()`). Es un túnel unidireccional de RAM alojado en el SO. Tiene solo 2 extremos (File descriptors).
  - El extremo `0` es para LEER (Read).
  - El extremo `1` es para ESCRIBIR (Write).
- Un Pipe simple es **Unidireccional**. Si el Padre quiere Hablarle al Hijo, y el Hijo quiere Responderle al Padre, ¡necesitas **DOS pipes** independientes! 
  *(Intentar leer y escribir del mismo `pipe()` a la vez desde dos clones es una receta garantizada para inter-bloqueos fatales, conocidos como "Deadlocks").*

**Paso Crítico: Cerrar lo que no se usa**
Si el Padre envía datos, debe cerrar (con `close()`) su Extremo Lector del tubo, porque si olvida cerrarlo, el Hijo jamás recibirá la señal mágica de *EOF* (Cierre de Archivo) cuando el padre termine, el Hijo se quedará esperando la eternidad a que alguien (incluido el padre) siga metiendo texto por un tubo abierto. Un Pipe solo se declara "Cerrado/Caído" cuando **TODOS** los procesos que tenían acceso a su extremo escritor hacen `close()`.

## 📝 Instrucciones

Construye `src/main.c` para entablar un "Ping-Pong" entre Padre e Hijo:
1. Crea 2 pipes (`fd_hacia_hijo[2]` y `fd_hacia_padre[2]`). 
2. Haz `fork()`.
3. El **Hijo**:
   - Cierra sus extremos inútiles (`fd_hacia_hijo[1]` y `fd_hacia_padre[0]`).
   - Usa `read()` del `fd_hacia_hijo[0]` para recibir un string del padre.
   - Lo imprime en consola, lo modifica de alguna forma (añadiéndole "Y el Hijo responde!"),
   - Usa `write()` y lo manda de vuelta por `fd_hacia_padre[1]`.
   - Cierra lo que queda, y hace `exit(0)`.
4. El **Padre**:
   - Cierra sus extremos inútiles (contrarios a los del hijo).
   - Hace `write()` mandándole una misión al `fd_hacia_hijo[1]`.
   - Hace `read()` quedándose pasmado y suspendido hasta que el Hijo empaquete y envíe un retorno mágico.
   - Lee, lo imprime en consola demostrando que el mensaje surfeó 2 veces los abismos del Kernel de Linux.
   - Llama a `wait()` por limpieza formal.

## ✅ Criterios de Éxito
- Padre transmite string -> Hijo Recibe String -> Hijo devuelve String Concatenado -> Padre imprime en Stdout final. Ningún pipe se saturará si cierras debidamente las bocas cruzadas.
