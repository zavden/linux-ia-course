#include <stdio.h>
#include <stdlib.h>

/*
 * Niveles de log numéricos para comparaciones simples.
 */
enum {
    LOG_ERROR = 1,
    LOG_INFO = 2,
    LOG_DEBUG = 3,
};

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_INFO
#endif

static int emitted_error = 0;
static int emitted_info = 0;
static int emitted_debug = 0;

/*
 * Cada macro decide en compile-time/runtime si imprime o no.
 * Así evitamos ensuciar código de negocio con ifs repetidos.
 */
#define LOGE(fmt, ...)                              \
    do {                                            \
        if (LOG_LEVEL >= LOG_ERROR) {              \
            emitted_error++;                        \
            printf("[ERR] " fmt "\n", __VA_ARGS__); \
        }                                           \
    } while (0)

#define LOGI(fmt, ...)                              \
    do {                                            \
        if (LOG_LEVEL >= LOG_INFO) {               \
            emitted_info++;                         \
            printf("[INF] " fmt "\n", __VA_ARGS__); \
        }                                           \
    } while (0)

#define LOGD(fmt, ...)                              \
    do {                                            \
        if (LOG_LEVEL >= LOG_DEBUG) {              \
            emitted_debug++;                        \
            printf("[DBG] " fmt "\n", __VA_ARGS__); \
        }                                           \
    } while (0)

int main(void) {
    /* ERROR debe imprimirse siempre que LOG_LEVEL >= 1. */
    LOGE("code=%d", 10);
    /* INFO se imprime en nivel 2 (default del ejercicio). */
    LOGI("step=%d", 1);
    LOGI("step=%d", 2);
    /* DEBUG no debería imprimirse con LOG_LEVEL=2. */
    LOGD("detail=%d", 99);

    /* Conteo final para validar qué rutas de macro se ejecutaron. */
    printf("level=%d err=%d info=%d dbg=%d\n", LOG_LEVEL, emitted_error, emitted_info, emitted_debug);

    return (LOG_LEVEL == LOG_INFO && emitted_error == 1 && emitted_info == 2 && emitted_debug == 0)
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
