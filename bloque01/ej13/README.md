# Ejercicio 1.3 — Structs y Listas Enlazadas Genéricas

## 🎯 Objetivo
En C no existen clases ni contenedores nativos (como `std::vector` en C++ o `list` en Python). Aquí aprenderemos a implementar nuestra propia estructura de datos dinámica y genérica usando `structs`, punteros y asestando nuestro conocimiento en `malloc` y `free`.

## 📚 Teoría Mínima
Una **Lista Enlazada Simplemente** (Singly Linked List) está compuesta por Nodos. Cada nodo contiene:
- Datos (el valor que guardas).
- Un puntero "siguiente" (`next`) que apunta a dónde vive el próximo Nodo en la memoria RAM.

Para hacerla "**Genérica**", el puntero de datos no será de tipo `int` o `char`, sino `void *`.
Un `void *` es un puntero mágico que puede apuntar a *cualquier* tipo de dato. Así nuestra lista podrá guardar desde enteros hasta structs hiper-complejos.
Al extraer el dato, el usuario de la lista hará un "cast" manual al tipo original.

## 📝 Instrucciones

1. He dejado la definición de la lista y sus funciones en `src/llist.h`.
2. Tu tarea es rellenar la implementación en `src/llist.c`:
   - `llist_create`: Debe retornar una lista vacía.
   - `llist_push`: Debe instanciar un nuevo `llist_node` con `malloc`, asignarle el dato genérico, y engancharlo **al inicio o al final** de la lista (tú decides, pero actualiza la cabeza).
   - `llist_pop`: Quita y devuelve el dato del primer nodo, usando `free` sobre la cajita del nodo.
   - `llist_destroy`: Recorre todos los nodos liberándolos y finalmente libera la cabeza de la lista.

3. He dejado código en `src/main.c` que usará tu librería para almacenar números enteros.

## ✅ Criterios de Éxito
- Al correr el test, los datos entran y salen correctamente sin generar Segfaults.
- Correr el test bajo Valgrind (`valgrind ./tests/test_llist.sh`) arroja 0 leaks. Las listas enlazadas son una trampa mortal de Memory Leaks; debes liberar (`free`) *absolutamente todo* nodo sacado o destruido.
