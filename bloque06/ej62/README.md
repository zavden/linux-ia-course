# Ejercicio 6.2 — El Origen de NodeJS: I/O No-Bloqueante (`fcntl`)

## 🎯 Objetivo
Evitar que tu Servidor o Programa se cuelgue en un temible y eterno `read()` o `accept()`. Transformaremos un descriptor de archivo "Terrestre Bloqueante", a uno **"Asíncrono/Flash No-Bloqueante"**, la base de toda la red moderna de alto rendimiento.

## 📚 Teoría Mínima
- Todo es un Archivo (Descriptor `int fd`): Teclado `0`, Pantalla `1`, Sockets `4`. 
- `read(fd)` es una función secuestradora. Si no hay teclado mecanografiado aún... tu App se dormirá (suspendida en estado S de Kernel I/O) y perderá el CPU sin importar qué más tuviera que hacer tu C!.
- La magia surge del `#include <fcntl.h>` (File Control). 
- **La Conversión Mística:** Le pides a Unix que modifique la bandera en caliente de ese fd: `int flags = fcntl(fd, F_GETFL, 0); fcntl(fd, F_SETFL, flags | O_NONBLOCK);`
- **¿Y ahora qué pasa con `read()`?**: Que si invocas read sobre un socket y suelta que todavía el Chino remoto y lento no te ha dado ningún enter ni byte... **`read` falla al instante y retorna explícitamente `-1` (o a veces `0`), ¡y PERO tu Código C continua de largo rapidísimo al `if` de la línea inferior!** 
- Sabrás qué fue este evento "Milagroso" porque la global de OS de Errores `errno` se seteará a **`EAGAIN`** o **`EWOULDBLOCK`** (Que en humano significa: *"Papi, todavía no baja tu byte desde la antena de WiFi al RAM, ¡sigue trabajando en otras cosas y pregúntame lueguito (Otra vuelta de Event-Loop)!"*).

## 📝 Instrucciones

Construye `src/main.c`. 
Vamos a leer el teclado crudo `STDIN_FILENO (0)` **SIN BLOQUEARNOS**!.
1. Imprime `Cambiando el Teclado Linux STDIn a MODO FLASH Async...`.
2. Extrae sus banderas `fcntl(STDIN_FILENO, F_GETFL, ...)` y reasígnalas `... | O_NONBLOCK`.
3. Entra a un `while(1)` infinito (El famosísimo **Event Loop** central).
4. Haz `sleep(1)` para no quemar el 100% de la carga del CPU con un while true agresivo!. Imprime `"."` como un reloj cargando.
5. Intenta leer el teclado: `int res = read(STDIN_FILENO, buf, sizeof(buf));`.
6. **El Control de Milagros**:
   - `if (res > 0)`: Has tecleado velozmente! Imprime su valor!. Si tipeaste "exit", haz de un Break pacífico para terminar y devolverlo a Blocking formal C.
   - `if (res < 0)`: Aquí la magia. Checa `if (errno == EAGAIN || errno == EWOULDBLOCK)`. Si lo es, Significa que `read` FRACASÓ maravillosamente protegiéndonos de Bloquear la Máquina!. Imprime "(Nada escrito todavía... sigo vivo calculando la UI o enviando el correo de fondo etc!)".

## ✅ Criterios de Éxito
- Has ejecutado un binario interactivo donde puedes ver de fondo un Loop de Reloj `...` transcurrir la vida libremente, y al mismo tiempo que aprietas tu teclado reacciona en la misma pantalla terminal cruzando Asincronismo I/O perfecto (Y en Single-Thread!! Sin paralelismos mentirosos Posix de C++ ni Forks costosos!).
