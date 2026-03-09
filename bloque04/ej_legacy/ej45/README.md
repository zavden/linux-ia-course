# Ejercicio 4.5 — Maestría en la Memoria: Custom Pool Allocator

## 🎯 Objetivo
Emular en C crudo a los lenguajes de alto nivel como Java o C#. Entender que llamar a `malloc` millones de veces seguidas para crear variables pequeñitas es **abismalmente lento**. Así que crearás tu propia "Memory Pool", un repartidor de memoria pre-reservada gigantesca para repartir *Chunks* minúsculos en nano-segundos.

## 📚 Teoría Mínima
- `malloc()` es un algoritmo complejo. Cuando le pides 10 bytes, él baja al sistema, busca un trozo disponible sin fragmentar y luego le añade *metadatos* (bytes extras escondidos antes de tu puntero) al inicio con información de "cuánto mide" por si llegases a hacerle `free`. Imagina sobrecargar millones de `malloc(2 bytes)`... consumirás el triple por pura basura de metadatos lentos.
- **La Solución (Pool Allocation):** 
1. Pides un INMENSO Y ÚNICO pedazo de 2 Megabytes al OS (`malloc(2000000)`). ¡Esto es super veloz! Sucedió 1 sola Syscall.
2. Cada vez que tu programa internamente instancie un "Soldadito o un Bloquecito", en vez de usar malloc, tú le entregas a tu App un puntero desplazado de tu gran piscina (Ej: `&piscina[usados]`), y actualizas matemáticamente tu Offset `(usados += tamanio)`.
3. Tu repisa interna no guarda Metadatos, y reparte los pedazos a la velocidad de CPU 1 ciclo!!.

## 📝 Instrucciones

Construye `src/main.c`.
1. Fija tu `struct pool_t`:
   `void *base; // El puntero del malloc gigante`.
   `size_t offset; // Cuántos bytes has rentado ya`.
   `size_t capacity; // El tamaño de la Gran Piscina`.
2. Crea `pool_create(size_t tam)` que llene e inicialize esa struct usando sólo **1** malloc real.
3. Crea `void *pool_alloc(pool_t *p, size_t n_pedazo)`. 
   - Debe sumar tu Offset.
   - Si Offset choca contra Capacity: Devolver `NULL` (Oop! tu propia Heap ficticia se llenó).
   - Si hay espacio, retornar `(char*)p->base + antiguo_offset`. ¡Felicidades, te auto-escribiste tu primer allocator O(1)!.
4. Ejecuta un Loop simulando C# o Unity: En una iteración inmensa de "1 Millón", reparte usando `pool_alloc` e intercala `malloc`. 
5. Si quieres lucirte: mides el tiempo de ambos (`clock()` es útil aquí) comprobando empíricamente tu superioridad.
6. Función `pool_destroy` que haga el **ÚNICO** free de esta inmensa piscina liberando ¡el millón de basuritas y datos huérfanos DE UN SIMPLE TIRO MATEMÁTICO!.

## ✅ Criterios de Éxito
- Tendrás en tu matriz un Memory Allocator funcional O(1) para Nodos que barre el piso frente al Malloc Glibc interno por su absoluta fragmentación 0, liberándote de las pesadillas de los Double-Free e inaugurando tus proezas de Senior C Coder.
