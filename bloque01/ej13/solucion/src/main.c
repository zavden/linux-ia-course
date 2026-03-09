/*
 * Ejercicio 1.3 — main.c usando lista genérica (SOLUCIÓN)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "llist.h"

int main(void) {
    llist_t *mi_lista = llist_create();

    // Guardaremos punteros a diferentes cosas. 
    // Data local en el stack (valida en este scope)
    int n1 = 100, n2 = 200;
    
    // Data dinámica en el heap
    char *texto = malloc(50);
    strcpy(texto, "Hola Mundo dinámico");

    printf("=== Llenando (LIFO / Pila) ===\n");
    llist_push(mi_lista, &n1);
    llist_push(mi_lista, &n2);
    llist_push(mi_lista, texto);
    
    printf("Hay %d elementos.\n", mi_lista->size);

    printf("\n=== Vaciando ===\n");
    // Sabemos empíricamente el orden inverso
    
    // 1. Sale 'texto'
    char *vTexto = (char*)llist_pop(mi_lista);
    if (vTexto) {
        printf("Salió: %s\n", vTexto);
        free(vTexto); // Responsabilidad NUESTRA, no de la lista
    }
    
    // 2. Sale n2 y n1
    int *vInt2 = (int*)llist_pop(mi_lista);
    int *vInt1 = (int*)llist_pop(mi_lista);
    
    if (vInt2) printf("Salió: %d\n", *vInt2);
    if (vInt1) printf("Salió: %d\n", *vInt1);

    // Destrucción
    llist_destroy(mi_lista);
    printf("Lista destruida limpiamente.\n");

    return EXIT_SUCCESS;
}
