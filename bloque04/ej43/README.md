# Ejercicio 4.3 — Telepatía Interprocesos: Memoria Compartida POSIX

## 🎯 Objetivo
Lograr el pináculo de la velocidad IPC (Inter-Process Communication). Olvídate de los estrechos cuellos de botella de los Pipes (`|`). Vamos a apartar un inmenso bloque de Memoria RAM en el núcleo de Linux, con el cual el Padre y el Hijo interactuarán al mismo tiempo sin trabas ni demoras.

## 📚 Teoría Mínima
- Las tuberías (pipes) obligan a Linux a **copiar** (`read()`, `write()`) bytes de la memoria del padre hacia el Kernel, y luego del Kernel hacia la memoria del hijo. Esa doble-copia desgasta el CPU si manejas un vídeo en 4K.
- La Memoria Compartida (`shm`) reserva un trozo vacío de silicón en la RAM, y conecta misteriosamente los punteros virtuales internos de **AMBOS** PROCESOS apunntanto Físicamente hacia esa mismísima lámina de Hardware. Cero sys-calls de read/write.
- En POSIX, se usa un "Archivo fantasma" en RAM (`/dev/shm` montura oculta de OS).
- Se crea con `int fd = shm_open("/mi_compartido", O_CREAT | O_RDWR, 0666)`.
- En crudo, `shm_open` crea un charco de **0 bytes**. Así que se debe usar `ftruncate(fd, 4096)` para dilatar el tamaño e "inflar" el charco de la RAM dándole un peso en Gigabytes o Bytes según se ocupe a costa del Sistema Operativo!.
- Al final, el Padre y el Hijo piden el clásico `mmap()` al mismo FD.

## 📝 Instrucciones

Construye `src/main.c`.
1. Fija un buffer de comunicación de al menos 4KB o un Array global gigante simple.
2. Padre: abre la memoria compartida `shm_open("/mi_memoria", O_CREAT|O_RDWR, 0666)`.
3. Trúncala (`ftruncate`) a tu buffer predeterminado (ej: 1024 bytes). En C se debe mapear `mmap(..., fd, ...)` como un array de *chars*.
4. Haz `fork()`.
5. En el **Padre**: 
   - Escribe un saludo inmenso ("¡Hola Hijo te veo directo a los ojos!") al array nativo C que te dio mmap: `strcpy(data, "...")`
   - Haz un clásico `waitpid()` suspendiéndote.
   - Cuando vuelvas del wait... ¡Imprime a pantalla en C esa misma variable global `data`! Milagrosamente notarás que las palabras "te veo a los ojos" ya no estarán sino que en el ínterin tu clon hijo escribió otra cosa y la RAM brilló hasta ti!
   - Limpia cerrando (`munmap`, `close`, y el asesino y desvinculador nuclear de OS: `shm_unlink("/mi_memoria")` para devolverle sus Bytes físicos al gobierno Kernel).
6. En el **Hijo**:
   - Duerme un segundo `sleep(1)` asegurándote de darle tiempo asíncrono rústico al Papa a llenar su strcpy original.
   - Accede sin compasión al mismo puntero (`data`). Haz un `printf` mostrándole a la terminal lo que el papa escribió! (Telepatía cruda cruzada de dos procesos C distintos en Linux que no tienen Variables compartidas).
   - Escríbele devuelta: `strcpy(data, "He leído tu mente papà, respondiendo.");`
   - Finaliza pacífico (`exit(0)`).

## ✅ Criterios de Éxito
- Te maravillarás ejecutando un script sin variables globales donde dos procesos C disidentes corren al mismo tiempo mutando strings lejanos instantáneamente de ida y vuelta engañando el aislamiento Virtual del Kernel (`MMU`).
