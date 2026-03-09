#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    /* Inicio del bloque grande administrado por el pool. */
    unsigned char *base;
    /* Tamaño total del pool en bytes. */
    size_t capacity;
    /* Próximo byte libre (bump pointer). */
    size_t offset;
} pool_t;

static int pool_init(pool_t *p, size_t cap) {
    /* Una sola reserva grande; luego alloc será solo aritmética de punteros. */
    p->base = malloc(cap);
    if (!p->base) return -1;
    p->capacity = cap;
    p->offset = 0;
    return 0;
}

/*
 * pool_alloc:
 * Retorna bloque contiguo de tamaño n.
 * No soporta free individual.
 */
static void *pool_alloc(pool_t *p, size_t n) {
    if (n == 0) return NULL;

    /* Chequeo de overflow y capacidad restante antes de avanzar offset. */
    if (p->offset > p->capacity || n > p->capacity - p->offset) {
        return NULL;
    }

    /* Dirección de salida dentro del bloque contiguo. */
    void *out = p->base + p->offset;
    /* "Consumimos" n bytes para la próxima asignación. */
    p->offset += n;
    return out;
}

static void pool_destroy(pool_t *p) {
    free(p->base);
    p->base = NULL;
    p->capacity = 0;
    p->offset = 0;
}

int main(void) {
    pool_t pool;
    /* Pool de 1 MiB para simular alta frecuencia de objetos chicos. */
    if (pool_init(&pool, 1024 * 1024) == -1) {
        perror("pool_init");
        return EXIT_FAILURE;
    }

    int ok = 0;
    for (int i = 0; i < 1000; ++i) {
        /* Asignaciones pequeñas simulando objetos frecuentes */
        char *slot = pool_alloc(&pool, 64);
        if (!slot) break;
        /* Inicializamos para demostrar que el bloque es escribible. */
        memset(slot, 0, 64);
        ok++;
    }

    printf("alloc_ok=%d used=%zu capacity=%zu\n", ok, pool.offset, pool.capacity);

    pool_destroy(&pool);
    return EXIT_SUCCESS;
}
