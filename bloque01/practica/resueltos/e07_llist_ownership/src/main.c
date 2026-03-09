#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Estructura de lista genérica */
typedef struct node {
    void *data;
    struct node *next;
} node_t;

typedef struct {
    node_t *head;
} list_t;

typedef void (*free_fn)(void *);

static char *dup_cstr(const char *s) {
    /* strdup no es ISO C puro en algunos entornos; implementamos una versión simple */
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (!p) return NULL;
    memcpy(p, s, n);
    return p;
}

static list_t *list_create(void) {
    list_t *l = malloc(sizeof(*l));
    if (!l) return NULL;
    l->head = NULL;
    return l;
}

static int list_push_front(list_t *l, void *data) {
    node_t *n = malloc(sizeof(*n));
    if (!n) return -1;
    n->data = data;
    n->next = l->head;
    l->head = n;
    return 0;
}

/*
 * Destroy extendido:
 * - libera nodos SIEMPRE
 * - libera data solo si se provee callback
 */
static void list_destroy_ex(list_t *l, free_fn fn) {
    if (!l) return;

    while (l->head) {
        node_t *n = l->head;
        l->head = n->next;

        if (fn) {
            fn(n->data);
        }

        free(n);
    }

    free(l);
}

int main(void) {
    list_t *list = list_create();
    if (!list) {
        perror("list_create");
        return EXIT_FAILURE;
    }

    char *s1 = dup_cstr("uno");
    char *s2 = dup_cstr("dos");
    if (!s1 || !s2) {
        perror("dup_cstr");
        free(s1);
        free(s2);
        list_destroy_ex(list, NULL);
        return EXIT_FAILURE;
    }

    if (list_push_front(list, s1) != 0 || list_push_front(list, s2) != 0) {
        perror("list_push_front");
        free(s1);
        free(s2);
        list_destroy_ex(list, NULL);
        return EXIT_FAILURE;
    }

    puts("lista creada y poblada");

    /* Aquí la lista SÍ es dueña del data y lo libera con free */
    list_destroy_ex(list, free);

    puts("ok");
    return EXIT_SUCCESS;
}
