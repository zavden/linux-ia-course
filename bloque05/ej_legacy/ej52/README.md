# Ejercicio 5.2 — Choque de Trenes Universales: Mutexes vs Race Conditions

## 🎯 Objetivo
Provocar intencionadamente un desastre estadístico (Race Condition) haciendo que múltiples hilos sumen locamente la misma variable al mismo tiempo y se arruinen entre sí. Luego, usaremos el milagro moderno de `pthread_mutex_t` (Locks) para blindar la zona y hacer tu código Thread-Safe al precio de paralizar el rendimiento.

## 📚 Teoría Mínima
- `contador++` NO es algo mágico u "atómico" en la P.C. Requiere **3 pasos**: (1) Ir a RAM a leerlo `[LOAD]`, (2) sumarle uno en el núcleo CPU `[ADD]`, (3) grabarlo de vuelta a la RAM `[STORE]`.
- Si 2 hilos hacen esto, el hilo A puede ir en el paso (2), ¡Y el hilo B le quita el control, hace el 1, 2 y 3 rápido guardando `101`, y luego el hilo A vuelve de la nada y hace su último paso de STORE clavando también `101`! Has sumado 2 veces y la PC registró 1 solo avance matemático.
- `pthread_mutex_t candado = PTHREAD_MUTEX_INITIALIZER;`
- Entras al peligro: `pthread_mutex_lock(&candado);`
- Haces `contador++`. (Si el hilo B quiso entrar, `lock` literalmente le congela el CPU y lo pausa eternamente, hasta que A...)
- Finalmente lo liberas abriendo puerta al B que esperaba ansioso en línea india: `pthread_mutex_unlock(&candado);`

## 📝 Instrucciones

Construye `src/main.c`.
1. Crea un Int Master `long int banco_master = 0`.
2. Haremos un `for` de creación `pthread_create` despachando **10 Hilos**.
3. El Hilo Misión: Su chamba es hacer un `for (1 to 1_000_000)` y sumar `banco_master++`.
4. Esperalos con Join y... plop. ¡Imprime el Log!. Por pura maldad de leyes de Murphy tu total resultará como "4,300,103" en vez del "10,000,000" prometido (¡Perdiste 5 millones de números por Race Conditions!).
5. **ARREGLÁNDOLO**: Repite el experimento. Pon la declaración de un `pthread_mutex_t`. Y ANTES de sumar y DESPUÉS de sumar, haz Lock y Unlock. Ahora se tomará como el quíntuple de tiempo de demora porque pusiste a los 10 hilos a hacer una humillante fila de cajero humano... pero ¡Oh Sorpresa, tu Banco reportará 10,000,000 sin desfalcos!.
6. (Opcional): Demuestra un segundo `for` rápido y peligroso (Benchmark) demostrándole al mundo por qué los Mutex, aunque salvan vidas, son el enemigo #1 del Multithreading eficiente por su cuello de botella atroz.

## ✅ Criterios de Éxito
- Tendrás un Archivo de compilación en el que intencionalmente se pierden Millones de dólares asíncronos en una ronda.
- Seguido de una ronda protegida formalmente por Sincronización POSIX Multicore en la que logras un conteo perfecto a raya.
