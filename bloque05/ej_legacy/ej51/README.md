# Ejercicio 5.1 — Bienvenidos al Multiverso (pthreads Básico)

## 🎯 Objetivo
Hacer que tu código se subdivida y conquiste un Arreglo de enteros masivo simultáneamente usando todos los núcleos físicos de tu CPU logrando la mítica "Calculación Paralela".

## 📚 Teoría Mínima
- Cabecera: `#include <pthread.h>`. (¡Compilar obligatoriamente con la bandera `-pthread` en gcc!)
- `int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);`
   - Creas un ID: `pthread_t t1;`
   - Envías función: `void *mi_rutina(void *arg) { ... return NULL; }`
   - Llamas el create.
- No olvides esperar o atarlos al terminar con: `int pthread_join(pthread_t thread, void **retval);` ¡Sufrirás Memory Leaks o Crashes si el Main C acaba antes que ellos!

## 📝 Instrucciones

Construye `src/main.c`.
1. Fija tu constante `NUM_ELEMENTOS = 10000000` (10 Millones). E Inicializa la Lista de `int` en Ceros globalmente.
2. Determina el Número de Hilos `NUM_THREADS = 4`.
3. Crea un Estructura `struct args_t` que contenga un entero a un `start_index` y un `end_index`.
4. En el `main()`, dispara un `for` instanciando **4 Threads**, pasándole a cada uno dinámicamente el fragmento de lista a manipular y rellenar. (Hilo 1 le envías de 0 a 2.5 Millones... etc.). Ten cuidado: Usa un array de argumentos diferente para cada iteración o todos los hilos pisarán el mismo puntero!.
5. La función Hija: Deberá tomar el puntero nulo Void, castearlo `(args_t*)` y hacer un simple for de Start a End guardando el índice de i * 2 en el array Master global.
6. En el `main()`, tras el for de Creations, haz un `for` de `pthread_join()` esperando a tus 4 guerrilleros.
7. Opcional: Para corroborar la veracidad mide los Clocks C antes y depues y comparalo corriendo un bucle tradicional SECUENCIAL.

## ✅ Criterios de Éxito
- Dispararás un programa masivo y verás a través de tu terminal local que los 4 sub-procesos finalizaron y tu Array ha sido llenado exitósamente logrando evadir corrupciones o memory violations. (Este ejercicio no da Race Condition porque los Índices que le pasaste están separados a propósito y los hilos no chocan la misma var a la vez).
