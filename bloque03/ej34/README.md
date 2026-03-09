# Ejercicio 3.4 — Tiro al Blanco: Señales y `sigaction`

## 🎯 Objetivo
Hacer que tu programa no explote miserablemente cuando un usuario ansioso spamea `Ctrl+C` en su teclado. Vamos a sobrevivir disparos de SIGINT, bloquear temporalmente señales en secciones críticas, y comunicarnos con nosotros mismos usando `SIGUSR1`.

## 📚 Teoría Mínima
- Las señales son interrupciones de software del SO. 
- Te han dicho en la universidad "Usa la función `signal()` para atrapar un Crtl+C". ¡NO LO HAGAS! `signal()` es una reliquia antiquísima pre-POSIX, sucia e impredecible entre arquitecturas (a veces se resetea sola tras atrapar).
- Usa **`sigaction()`**: Te permite bloquear *otras* señales MIENTRAS ya estás manejando esta, impide que te vuelvan a disparar el mismo evento antes de que termines, y se queda activada para siempre con fiabilidad quirúrgica moderna.
- Variables volátiles asíncronas: Como los manejadores ("handlers") de señales se ejecutan "de la nada" congelando el programa local, la única forma de que modifiquen una variable y el bucle principal se dé cuenta, es que la variable sea global, de tipo `sig_atomic_t` (que resiste lecturas partidas), y de pre-fijo `volatile` (para que el compilador no trate de esconder su valor en un registro veloz asumiendo erróneamente que "nunca cambia").

## 📝 Instrucciones

1. Crea variables globales `volatile sig_atomic_t` necesarias. (Por ej: un booleano para salir y un contador para los SIGINT).
2. Crea un handler (funcion `void mi_handler(int sig)`) que cuando sea `SIGINT`, incremente el contador, e imprima que faltan disparos para la muerte a menos que el contador llegue a 3, en cuyo caso setea el flag global de salida `flag_run = 0`. Y si es `SIGUSR1`, que imprima un saludo exótico, y si es `SIGTERM` (Muerte Limpia), fuerce salida pacífica.
3. En el `main()`, registra ESE handler para todas estas señales. Requerirás setear `sa_flags = 0` o `SA_RESTART` y limpiar todo con una máscara de bloqueo de interrupciones (`sigemptyset`).
4. Haz una "Sección Crítica" con la API **`sigprocmask`**: 
   Añade `SIGINT` temporalmente a las "señales bloqueadas" del Linux (las atrapará pero te las dará al salir, retrasándolas y poniéndolas en Stand-by para no arruinar tu código delicado de 5 segundos donde duermes tranquilo).
5. Desbloquéalo y por fin entra en el un infinito `while(flag_run)` haciendo `sleep`.

## ✅ Criterios de Éxito
- Lanzas esto libremente en bash, presionas Crtl+C como un loco, y te dice "Intenta más suerte la próxima... 1/3, 2/3... 3/3 -> Adiós". Y `kill -USR1 <PID>` te sorprende con un mensaje oculto.
