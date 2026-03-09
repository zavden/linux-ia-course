#include "text_stats.h"

#include <ctype.h>

/*
 * Cuenta vocales y consonantes ASCII básicas.
 * Ignora cualquier carácter que no sea letra.
 */
void text_count_vowels_consonants(const char *s, int *out_vowels, int *out_consonants) {
    int vowels = 0;
    int consonants = 0;

    for (size_t i = 0; s[i] != '\0'; ++i) {
        unsigned char ch = (unsigned char)s[i];
        if (!isalpha(ch)) {
            continue;
        }

        ch = (unsigned char)tolower(ch);
        if (ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u') {
            vowels++;
        } else {
            consonants++;
        }
    }

    *out_vowels = vowels;
    *out_consonants = consonants;
}
