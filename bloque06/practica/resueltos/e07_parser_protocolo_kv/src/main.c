#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ITEMS 16

typedef struct {
    /* used=1 indica slot ocupado con clave válida. */
    int used;
    char key[32];
    char value[96];
} kv_t;

static kv_t db[MAX_ITEMS];

/*
 * Busca índice por clave.
 * Retorna -1 si no existe.
 */
static int find_key(const char *key) {
    for (int i = 0; i < MAX_ITEMS; ++i) {
        if (db[i].used && strcmp(db[i].key, key) == 0) {
            return i;
        }
    }
    return -1;
}

/*
 * Busca primer slot libre para inserción.
 */
static int find_free_slot(void) {
    for (int i = 0; i < MAX_ITEMS; ++i) {
        if (!db[i].used) {
            return i;
        }
    }
    return -1;
}

/*
 * Procesa un comando textual y escribe respuesta en out.
 * Protocolo:
 * - SET <key> <value>
 * - GET <key>
 * - DEL <key>
 */
static void handle_command(const char *line, char *out, size_t out_sz) {
    char cmd[8] = {0};
    char key[32] = {0};
    char val[96] = {0};

    int n = sscanf(line, "%7s %31s %95s", cmd, key, val);
    if (n <= 0) {
        snprintf(out, out_sz, "ERR");
        return;
    }

    if (strcmp(cmd, "SET") == 0) {
        if (n != 3) {
            snprintf(out, out_sz, "ERR");
            return;
        }

        int idx = find_key(key);
        if (idx == -1) {
            idx = find_free_slot();
            if (idx == -1) {
                /* Sin espacio disponible en almacenamiento fijo. */
                snprintf(out, out_sz, "ERR FULL");
                return;
            }
            db[idx].used = 1;
            snprintf(db[idx].key, sizeof(db[idx].key), "%s", key);
        }

        snprintf(db[idx].value, sizeof(db[idx].value), "%s", val);
        snprintf(out, out_sz, "OK");
        return;
    }

    if (strcmp(cmd, "GET") == 0) {
        if (n != 2) {
            snprintf(out, out_sz, "ERR");
            return;
        }

        int idx = find_key(key);
        if (idx == -1) {
            snprintf(out, out_sz, "NULL");
        } else {
            snprintf(out, out_sz, "VALUE %s", db[idx].value);
        }
        return;
    }

    if (strcmp(cmd, "DEL") == 0) {
        if (n != 2) {
            snprintf(out, out_sz, "ERR");
            return;
        }

        int idx = find_key(key);
        if (idx != -1) {
            db[idx].used = 0;
            db[idx].key[0] = '\0';
            db[idx].value[0] = '\0';
        }
        snprintf(out, out_sz, "OK");
        return;
    }

    snprintf(out, out_sz, "ERR");
}

int main(void) {
    const char *cmds[] = {
        "SET user ana",
        "GET user",
        "DEL user",
        "GET user",
        "WHAT",
    };

    char resp[5][128];
    memset(resp, 0, sizeof(resp));

    for (int i = 0; i < 5; ++i) {
        /* Ejecutamos secuencia cerrada para validar parser extremo a extremo. */
        handle_command(cmds[i], resp[i], sizeof(resp[i]));
    }

    /* Salida compacta para test con grep y debugging rápido. */
    printf("resp0=%s resp1=%s resp2=%s resp3=%s resp4=%s\n",
           resp[0], resp[1], resp[2], resp[3], resp[4]);

    return (strcmp(resp[0], "OK") == 0 &&
            strcmp(resp[1], "VALUE ana") == 0 &&
            strcmp(resp[2], "OK") == 0 &&
            strcmp(resp[3], "NULL") == 0 &&
            strcmp(resp[4], "ERR") == 0)
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}
