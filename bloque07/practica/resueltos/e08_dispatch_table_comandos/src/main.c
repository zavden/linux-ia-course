#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int (*op_fn)(int a, int b);

typedef struct {
    const char *name;
    op_fn fn;
} command_t;

static int op_sum(int a, int b) { return a + b; }
static int op_sub(int a, int b) { return a - b; }
static int op_mul(int a, int b) { return a * b; }

static const command_t g_table[] = {
    {"sum", op_sum},
    {"sub", op_sub},
    {"mul", op_mul},
};

/* Busca operación por nombre en la tabla de dispatch. */
static op_fn find_op(const char *name) {
    /* Escaneo lineal: simple y claro para un número pequeño de comandos. */
    for (size_t i = 0; i < sizeof(g_table) / sizeof(g_table[0]); ++i) {
        if (strcmp(g_table[i].name, name) == 0) {
            return g_table[i].fn;
        }
    }
    return NULL;
}

/* Ejecuta comando textual "op a b". */
static int execute(const char *line, int *out) {
    char cmd[16] = {0};
    int a = 0;
    int b = 0;

    if (sscanf(line, "%15s %d %d", cmd, &a, &b) != 3) {
        return -1;
    }

    op_fn fn = find_op(cmd);
    if (!fn) {
        return -1;
    }

    *out = fn(a, b);
    return 0;
}

int main(void) {
    /* Secuencia de comandos para probar rutas válidas e inválidas. */
    const char *inputs[] = {
        "sum 3 4",
        "sub 9 2",
        "mul 2 5",
        "nop 1 1",
    };

    int r0 = 0, r1 = 0, r2 = 0, r3 = 0;
    int ok0 = execute(inputs[0], &r0);
    int ok1 = execute(inputs[1], &r1);
    int ok2 = execute(inputs[2], &r2);
    int ok3 = execute(inputs[3], &r3);

    /* r0/r1/r2 validan rutas felices de dispatch + aritmética correcta. */
    /* r3 marca error esperado porque \"nop\" no existe en la tabla. */
    printf("r0=%d r1=%d r2=%d r3=%s\n", r0, r1, r2, (ok3 == -1) ? "ERR" : "OK");

    return (ok0 == 0 && ok1 == 0 && ok2 == 0 && ok3 == -1 && r0 == 7 && r1 == 7 && r2 == 10)
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
