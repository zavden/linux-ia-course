#include <stdio.h>
#include <stdlib.h>
#include "llist.h"

int main(void) {
    llist_t *mi_lista = llist_create();

    // Como es void*, podemos pasar la dirección de momeria de variables locales (aunque sea peligroso si salen del scope)
    int act1 = 100;
    int act2 = 200;
    
    // Test push
    llist_push(mi_lista, &act1);
    llist_push(mi_lista, &act2);

    // Test pop (debería salir 200 luego 100 si es LIFO/Pila)
    int *val1 = (int*)llist_pop(mi_lista);
    int *val2 = (int*)llist_pop(mi_lista);

    if(val1 && val2) {
        printf("Salió: %d, Luego: %d\n", *val1, *val2);
    }

    // Asegurarse de limpiar
    llist_destroy(mi_lista);
    return EXIT_SUCCESS;
}
