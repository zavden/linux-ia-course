#include <stdio.h>
#include <stdlib.h>

/*
 * Reto C02:
 * Diseña una lista genérica reusable de verdad.
 */

typedef struct node {
    void *data;
    struct node *next;
} node_t;

typedef struct {
    node_t *head;
    node_t *tail;
    size_t size;
} list_t;

typedef int (*pred_fn)(const void *elem, const void *ctx);
typedef void (*visit_fn)(void *elem, void *ctx);
typedef void (*free_fn)(void *elem);

/* TODO: create/destroy */
static list_t *list_create(void) {
    list_t *l = malloc(sizeof(*l));
    if (!l) return NULL;
    l->head = NULL;
    l->tail = NULL;
    l->size = 0;
    return l;
}

/*
 * TODO: push_front y push_back.
 * Recuerda actualizar tail cuando insertas en lista vacía.
 */

/*
 * TODO: pop_front.
 * Caso delicado: cuando sale el último nodo debes dejar head y tail en NULL.
 */

/*
 * TODO: find con predicado.
 * Debe devolver el primer elemento que cumpla la condición.
 */

/*
 * TODO: foreach para aplicar función a cada elemento.
 * Útil para imprimir o transformar.
 */

/*
 * TODO: destroy_ex.
 * Si free_fn != NULL, liberar también el data de cada nodo.
 */

int main(void) {
    list_t *l = list_create();
    if (!l) {
        perror("list_create");
        return EXIT_FAILURE;
    }

    /*
     * TODO:
     * 1) Inserta datos
     * 2) Recorre con foreach
     * 3) Busca con find
     * 4) Extrae con pop_front
     * 5) Libera con destroy_ex
     */

    puts("C02 plantilla lista para resolver");

    /* Temporal para no filtrar la lista de prueba vacía */
    free(l);
    return EXIT_SUCCESS;
}
