#include <stdio.h>
#include <stdlib.h>

#include "text_stats.h"

int main(void) {
    /* Texto de muestra para validar API exportada por la librería estática. */
    /* También verifica conteo case-insensitive básico. */
    const char *sample = "DebugMake";

    int vowels = 0;
    int consonants = 0;

    /* Llamada a función ubicada en libtextstats.a. */
    text_count_vowels_consonants(sample, &vowels, &consonants);

    /* Salida estable: útil para test y para verificar link correcto. */
    printf("text=%s vowels=%d consonants=%d\n", sample, vowels, consonants);

    /* Éxito si la métrica coincide con valor esperado del string fijo. */
    return (vowels == 4 && consonants == 5) ? EXIT_SUCCESS : EXIT_FAILURE;
}
