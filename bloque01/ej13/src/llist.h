#ifndef LLIST_H
#define LLIST_H

// Un nodo aislado de la lista
typedef struct llist_node {
    void *data;                 // Puntero genérico al dato real
    struct llist_node *next;    // Puntero al siguiente eslabón
} llist_node_t;

// La "cabeza" de la lista
typedef struct {
    llist_node_t *head; // El primer nodo
    int size;           // Cuántos hay
} llist_t;

// Crea la estructura base vacía
llist_t *llist_create(void);

// Inserta al *principio* de la lista (push)
void llist_push(llist_t *list, void *data);

// Saca y devuelve el *primer* elemento
void *llist_pop(llist_t *list);

// Libera todos los nodos y la propia lista
void llist_destroy(llist_t *list);

#endif
