# Ejercicio 4.4 — Cadenas del Kernel: Límite de Recursos (`getrlimit`)

## 🎯 Objetivo
Hacer que tu propio programa averigüe cuánta libertad le concede el Sistema Operativo para consumir hardware (RAM, Archivos Abiertos, etc) e intentar voluntariamente capar sus propias capacidades, auto-saboteándose e invocando al justiciero y letal *OOM Killer*.

## 📚 Teoría Mínima
- Todo proceso nace con "Límites Suaves" (`Soft Limit / rlim_cur`) y "Límites Duros" (`Hard Limit / rlim_max`).
- El humano que te invoca desde Bash, define en su consola estos topes llamando al comando OS `ulimit -a`.
- En C, usas la estructura `struct rlimit` junto a `getrlimit()` para leer tus sentencias y la syscall `setrlimit()` para cambiarlas de forma programática.
- Un proceso libre sin privilegios Root SIEMPRE PUEDE **bajar** sus límites suaves o duros (como un mártir asceta). Pero, una vez que baja voluntariamente o no su límite asfixiando sus capacidades duras... !Jamás lo podrá subir nuevamente por encima!. Sólo el root del sistema tiene derecho a alzar los "Max Limits" de un programa restringido.

## 📝 Instrucciones

Construye `src/main.c`.
1. Crea variables `struct rlimit` y lee los topes de FDs abiertos (File Descriptors) con la constante macro `RLIMIT_NOFILE`. Imprímelos por consola ("Soft: X, Hard: Y").
2. Lee los límites de Memoria Virtual o RAM global (`RLIMIT_AS` o Data `RLIMIT_DATA`) e imprímelos (usualmente si Linux es tu PC en lugar de un contenedor asfixiado saldrá `RLIM_INFINITY` (-1) denotando infinito consumible).
3. **El Sabojate**: 
   - Modifica intencionadamente y fuertemente la variable `struct rlimit r`. Ponle un máximo de Memos de digamos... 8 Megabytes (`8 * 1024 * 1024 bytes`).
   - Llama a `setrlimit(RLIMIT_AS, &r)`. Asegúrate de revisar que retornó `0` de que aceptó la castración del kernel.
4. **Prueba Kamikaze**:
   - Corre un bucle C maligno `for (int i=0; i<300; i++)` que constantemente meta llamadas en fila de `malloc(1 Megabyte)` puro a una pequeña lista encadenada. 
   - Haz que llene sus array de Ceros asumiendo escritura (`memset(buf, 0, 1MB)`), para forzar al Kernel a buscar el espacio. 
   - ¡CUIDANDO CÓDIGOS DE ERROR! Tu programa C detectará (si lo haces bien con un `if (!buf)`) que en el turno `~8` el infame `malloc` **Fracasó dándote `NULL`** porque el SO se rehusó dadas tus propias limitantes cortadas!.

## ✅ Criterios de Éxito
- Verás en los logs que el programa auto-infligió su asfixia y luego rompió fallando maravillosamente su expansión glibc local al tropezar de cara con "Cannot allocate Memory".
