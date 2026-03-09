#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    /* Byte stream acumulado aún no convertido a mensajes completos. */
    char buf[256];
    size_t len;
} line_buffer_t;

typedef struct {
    /* Cantidad de líneas completas detectadas. */
    int count;
    /* Primera línea para validar orden de llegada. */
    char first[128];
    /* Última línea para validar procesamiento total. */
    char last[128];
} collector_t;

/*
 * Callback invocado por cada línea completa detectada.
 */
static void on_line(const char *line, collector_t *c) {
    c->count++;
    if (c->count == 1) {
        snprintf(c->first, sizeof(c->first), "%s", line);
    }
    snprintf(c->last, sizeof(c->last), "%s", line);
}

/*
 * Alimenta bytes al buffer y extrae líneas terminadas en '\n'.
 * Mantiene en buf cualquier remanente incompleto para el próximo chunk.
 */
static int feed_bytes(line_buffer_t *lb, const char *data, size_t n, collector_t *c) {
    if (lb->len + n >= sizeof(lb->buf)) {
        return -1;
    }

    memcpy(lb->buf + lb->len, data, n);
    lb->len += n;

    size_t start = 0;
    for (size_t i = 0; i < lb->len; ++i) {
        if (lb->buf[i] == '\n') {
            char line[128];
            size_t llen = i - start;
            if (llen >= sizeof(line)) {
                return -1;
            }
            memcpy(line, lb->buf + start, llen);
            line[llen] = '\0';
            on_line(line, c);
            start = i + 1;
        }
    }

    /* Compactamos el remanente no procesado al inicio del buffer. */
    if (start > 0) {
        size_t rem = lb->len - start;
        memmove(lb->buf, lb->buf + start, rem);
        lb->len = rem;
    }

    return 0;
}

int main(void) {
    line_buffer_t lb;
    memset(&lb, 0, sizeof(lb));

    collector_t c;
    memset(&c, 0, sizeof(c));

    /* Stream fragmentado a propósito para simular TCP real. */
    const char *chunks[] = {
        "SET a",
        " 1\nGET",
        " a\nPIN",
        "G\n",
    };

    /* Simulamos recepción de red fragmentada en múltiples read(). */
    for (size_t i = 0; i < sizeof(chunks) / sizeof(chunks[0]); ++i) {
        if (feed_bytes(&lb, chunks[i], strlen(chunks[i]), &c) == -1) {
            fprintf(stderr, "feed_bytes failed\n");
            return EXIT_FAILURE;
        }
    }

    /* pending=0 confirma que no quedaron bytes huérfanos sin parsear. */
    printf("lines=%d first=%s last=%s pending=%zu\n", c.count, c.first, c.last, lb.len);

    return (c.count == 3 && strcmp(c.first, "SET a 1") == 0 && strcmp(c.last, "PING") == 0 && lb.len == 0)
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
