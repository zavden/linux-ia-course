#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// TODO 1: struct pool_t ...
typedef struct {
    void *base;
    size_t capacity;
    size_t offset;
} mem_pool;

// TODO 2: mem_pool* pool_create(size)
// TODO 3: void* pool_alloc(mem_pool*, size n)    
// TODO 4: void pool_destroy(mem_pool*)

int main(void) {
    // 1. Instanciar Pool gigantesca (ej. 1 Mega)
    // 2. Proteger checkeando Nulls
    // 3. Loop: Pedir (alloc) N bloquecitos desde la Pool de forma veloz.
    // 4. (Opcional) Loop Alterno: Pedir N bloquecitos de malloc real.
    // 5. pool_destroy para una limpieza absoluta milagrosa de toda tu listado.
    
    return EXIT_SUCCESS;
}
