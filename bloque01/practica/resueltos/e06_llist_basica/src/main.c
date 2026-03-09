#include <stdio.h>
#include <stdlib.h>

/* Nodo genérico: guarda puntero a dato y enlace al siguiente */
typedef struct node {
    void *data;
    struct node *next;
} node_t;

/* Lista con cabeza y tamaño para mantener invariantes */
typedef struct {
    node_t *head;
    int size;
} list_t;

static list_t *list_create(void) {
    list_t *l = malloc(sizeof(*l));
    if (!l) return NULL;
    l->head = NULL;
    l->size = 0;
    return l;
}

/* Inserción al frente: operación constante */
static int list_push_front(list_t *l, void *data) {
    node_t *n = malloc(sizeof(*n));
    if (!n) return -1;
    n->data = data;
    n->next = l->head;
    l->head = n;
    l->size++;
    return 0;
}

/* Extracción al frente: devuelve NULL si la lista está vacía */
static void *list_pop_front(list_t *l) {
    if (!l || !l->head) return NULL;

    node_t *n = l->head;
    void *data = n->data;

    l->head = n->next;
    free(n);
    l->size--;

    return data;
}

/*
 * Destroy solo libera nodos (no data), porque en este ejercicio
 * la lista NO es dueña de los elementos almacenados.
 */
static void list_destroy(list_t *l) {
    if (!l) return;

    while (l->head) {
        node_t *n = l->head;
        l->head = n->next;
        free(n);
    }

    free(l);
}

int main(void) {
    int a = 100;
    int b = 200;

    list_t *list = list_create();
    if (!list) {
        perror("list_create");
        return EXIT_FAILURE;
    }

    if (list_push_front(list, &a) != 0 || list_push_front(list, &b) != 0) {
        perror("list_push_front");
        list_destroy(list);
        return EXIT_FAILURE;
    }

    int *x = list_pop_front(list);
    int *y = list_pop_front(list);

    printf("%d %d size=%d\n", x ? *x : -1, y ? *y : -1, list->size);

    list_destroy(list);
    return EXIT_SUCCESS;
}
