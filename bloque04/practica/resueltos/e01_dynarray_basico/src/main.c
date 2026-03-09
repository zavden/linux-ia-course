#include <stdio.h>
#include <stdlib.h>

typedef struct {
    /* Puntero al bloque contiguo de enteros. */
    int *data;
    /* Cantidad de elementos realmente usados. */
    size_t len;
    /* Capacidad total reservada (en elementos, no bytes). */
    size_t capacity;
} dynarray_t;

/*
 * init:
 * Reserva memoria inicial para N enteros.
 * Devuelve 0 en éxito, -1 en fallo.
 */
static int dyn_init(dynarray_t *a, size_t initial_capacity) {
    /* Reservamos bloque inicial; len arranca en 0. */
    a->data = malloc(initial_capacity * sizeof(int));
    if (!a->data) {
        return -1;
    }
    a->len = 0;
    a->capacity = initial_capacity;
    return 0;
}

/*
 * grow:
 * Duplica capacidad con realloc.
 * Patrón seguro: usar temporal para no perder el puntero viejo en fallo.
 */
static int dyn_grow(dynarray_t *a) {
    /* Estrategia habitual: crecimiento geométrico para costo amortizado O(1). */
    size_t new_capacity = a->capacity * 2;
    int *tmp = realloc(a->data, new_capacity * sizeof(int));
    if (!tmp) {
        return -1;
    }
    a->data = tmp;
    a->capacity = new_capacity;
    return 0;
}

/*
 * push:
 * Inserta elemento al final.
 * Si no hay capacidad, realoca antes.
 */
static int dyn_push(dynarray_t *a, int value) {
    /* Si llenamos el buffer, realocamos antes de escribir. */
    if (a->len == a->capacity) {
        if (dyn_grow(a) == -1) {
            return -1;
        }
        /* Traza pedagógica: ver cuándo y a cuánto crece el arreglo. */
        printf("grow_to=%zu\n", a->capacity);
    }

    /* Escritura al final (append). */
    a->data[a->len] = value;
    /* Avanzamos longitud lógica del vector. */
    a->len++;
    return 0;
}

static void dyn_destroy(dynarray_t *a) {
    free(a->data);
    a->data = NULL;
    a->len = 0;
    a->capacity = 0;
}

int main(void) {
    dynarray_t a;
    /* Iniciamos con capacidad chica a propósito para forzar grows. */
    if (dyn_init(&a, 2) == -1) {
        perror("dyn_init");
        return EXIT_FAILURE;
    }

    /* Insertamos una secuencia simple para validar orden y realocación. */
    for (int i = 1; i <= 20; ++i) {
        if (dyn_push(&a, i) == -1) {
            perror("dyn_push");
            dyn_destroy(&a);
            return EXIT_FAILURE;
        }
    }

    /* Estado final esperado: len=20, capacidad >=20 y último valor=20. */
    printf("len=%zu cap=%zu last=%d\n", a.len, a.capacity, a.data[a.len - 1]);

    /* Cleanup obligatorio para no filtrar memoria heap. */
    dyn_destroy(&a);
    return EXIT_SUCCESS;
}
