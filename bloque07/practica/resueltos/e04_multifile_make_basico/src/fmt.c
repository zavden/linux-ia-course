#include "fmt.h"

#include <stdio.h>

/*
 * Formatea "label=value" en un buffer externo.
 * Devuelve 0 si entra completo, -1 si se truncaría.
 */
int fmt_pair(char *out, size_t out_sz, const char *label, int value) {
    int n = snprintf(out, out_sz, "%s=%d", label, value);
    if (n < 0 || (size_t)n >= out_sz) {
        return -1;
    }
    return 0;
}
