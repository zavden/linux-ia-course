/*
 * Ejercicio 1.3 — Listas Enlazadas Genéricas (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Aprender a manejar memoria dinámica en bruto (`malloc` / `free`) gestionando
 * nosotros mismos el ciclo de vida de los datos, usando punteros a `void` 
 * para construir un contenedor genérico.
 */

#include "llist.h"
#include <stdlib.h> // malloc, free
#include <stdio.h>  // perror

/*
 * 1. CREACIÓN DE LA LISTA
 * Se reserva memoria para el "gestor" o "cabeza" de la lista.
 * Esta estructura de control vivirá en el Heap.
 */
llist_t *llist_create(void) {
    // malloc: Dame espacio para alojar el tamaño de la estructura llist_t.
    llist_t *list = malloc(sizeof(llist_t));
    if (!list) {
        perror("Error de memoria creando lista");
        return NULL;
    }
    
    // Inicialización: La lista arranca vacía.
    list->head = NULL;
    list->size = 0;
    
    return list;
}

/*
 * 2. INSERCIÓN DE NODOS (Push - LIFO / Pila)
 * Añadimos un elemento nuevo por la "cabeza" de la lista por eficiencia O(1).
 */
void llist_push(llist_t *list, void *data) {
    if (!list) return; // Validación defensiva
    
    // Creamos la nueva "caja" (nodo) para envolver el dato.
    llist_node_t *new_node = malloc(sizeof(llist_node_t));
    if (!new_node) {
        perror("Error de memoria creando nodo");
        return;
    }
    
    // 2.A. Guardamos nuestro puntero genérico (void*)
    new_node->data = data;
    
    // 2.B. Enganche (El nuevo nodo apunta al que antes era el primero)
    new_node->next = list->head;
    
    // 2.C. Actualización (Ahora el nuevo nodo ES el primero)
    list->head = new_node;
    
    list->size++; // Contabilidad
}

/*
 * 3. EXTRACCIÓN DE NODOS (Pop - LIFO / Pila)
 * Quitamos el primer elemento destruyendo su nodo, pero devolviendo su data interior.
 */
void *llist_pop(llist_t *list) {
    // Si la lista es nula o está vacía, no devolvemos nada.
    if (!list || !list->head) return NULL;
    
    // Identificamos el nodo víctima (el primero).
    llist_node_t *victim = list->head;
    
    // Rescatamos la gema de su interior antes de destruirlo.
    void *item = victim->data;
    
    // Desvinculamos: El nuevo primer elemento será el segundo.
    list->head = victim->next;
    
    // ¡CRÍTICO! Destruimos la "caja" del nodo que alojamos con malloc en push().
    // Sin esto, tendríamos Memory Leaks.
    free(victim);
    
    list->size--; // Contabilidad
    
    // Retornamos el dato original al dueño
    return item;
}

/*
 * 4. DESTRUCCIÓN (Cleanup total)
 * Si el usuario se aburrió de la lista, debemos destruir todos los nodos
 * remanentes que no sacó con pop(), y finalmente destruir la cabeza de la lista.
 * 
 * NOTA: Esto destruye los "nodos", NO destruye el contenido (data) porque
 * al ser void*, no sabemos si `data` vino de un malloc() ajeno o de la pila local.
 * Es responsabilidad del programador que usa esta lista liberar los datos reales.
 */
void llist_destroy(llist_t *list) {
    if (!list) return;
    
    // Recorremos la estructura vaciándola usando pop()
    // Esto re-utiliza lógica, evita duplicar código y garantiza limpieza.
    while (list->head != NULL) {
        // Ignoramos el dato devuelto (memory leak temporal en caso de punteros puros externos).
        llist_pop(list); 
    }
    
    // Finalmente, la lista (structura de control) misma en sí
    free(list);
}
