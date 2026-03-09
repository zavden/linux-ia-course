#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int shares;
    int writable;
    int guest_ok;
    int browseable_no;
} smb_stats_t;

typedef struct {
    int active;
    int is_global;
    int seen_read_only;
    int read_only;
    int seen_writable;
    int writable;
    int guest_ok;
    int browseable;
} share_state_t;

static char *trim(char *s) {
    while (*s != '\0' && isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;

    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end-- = '\0';
    }
    return s;
}

static void to_lower_ascii(char *s) {
    for (size_t i = 0; s[i] != '\0'; ++i) {
        s[i] = (char)tolower((unsigned char)s[i]);
    }
}

/* Convierte valor textual yes/no/true/false en bool simple. */
static int parse_bool(const char *v, int *out) {
    char tmp[64];
    if (snprintf(tmp, sizeof(tmp), "%s", v) >= (int)sizeof(tmp)) {
        return -1;
    }
    to_lower_ascii(tmp);

    if (strcmp(tmp, "yes") == 0 || strcmp(tmp, "true") == 0) {
        *out = 1;
        return 0;
    }
    if (strcmp(tmp, "no") == 0 || strcmp(tmp, "false") == 0) {
        *out = 0;
        return 0;
    }

    return -1;
}

/*
 * Cierra analisis de share actual y acumula en estadisticas globales.
 */
static void flush_share(const share_state_t *sh, smb_stats_t *st) {
    if (!sh->active || sh->is_global) {
        return;
    }

    st->shares++;

    int writable_effective = 0;
    if (sh->seen_writable) {
        writable_effective = sh->writable;
    } else if (sh->seen_read_only) {
        writable_effective = sh->read_only ? 0 : 1;
    } else {
        /* Sin dato explicito, mantenemos default conservador (read-only). */
        writable_effective = 0;
    }

    if (writable_effective) st->writable++;
    if (sh->guest_ok) st->guest_ok++;
    if (!sh->browseable) st->browseable_no++;
}

int main(int argc, char **argv) {
    const char *path = (argc == 2) ? argv[1] : "tests/data/smb.conf.sample";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<smb_conf>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    smb_stats_t st = {0, 0, 0, 0};
    share_state_t cur = {0, 0, 0, 1, 0, 0, 0, 1};

    char line[1024];
    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';

        char *comment = strpbrk(line, "#;");
        if (comment) {
            *comment = '\0';
        }

        char *p = trim(line);
        if (*p == '\0') {
            continue;
        }

        /* Nueva seccion [name]. Antes cerramos la anterior. */
        if (p[0] == '[') {
            char *rb = strchr(p, ']');
            if (rb == NULL) {
                continue;
            }

            flush_share(&cur, &st);

            *rb = '\0';
            char name[128];
            snprintf(name, sizeof(name), "%s", p + 1);
            to_lower_ascii(name);

            cur.active = 1;
            cur.is_global = (strcmp(name, "global") == 0) ? 1 : 0;
            cur.seen_read_only = 0;
            cur.read_only = 1;
            cur.seen_writable = 0;
            cur.writable = 0;
            cur.guest_ok = 0;
            cur.browseable = 1;
            continue;
        }

        if (!cur.active) {
            continue;
        }

        char *eq = strchr(p, '=');
        if (eq == NULL) {
            continue;
        }

        *eq = '\0';
        char *key = trim(p);
        char *val = trim(eq + 1);

        char key_norm[128];
        snprintf(key_norm, sizeof(key_norm), "%s", key);
        to_lower_ascii(key_norm);

        int b = 0;
        if (strcmp(key_norm, "read only") == 0) {
            if (parse_bool(val, &b) == 0) {
                cur.seen_read_only = 1;
                cur.read_only = b;
            }
            continue;
        }

        if (strcmp(key_norm, "writable") == 0 || strcmp(key_norm, "writeable") == 0) {
            if (parse_bool(val, &b) == 0) {
                cur.seen_writable = 1;
                cur.writable = b;
            }
            continue;
        }

        if (strcmp(key_norm, "guest ok") == 0) {
            if (parse_bool(val, &b) == 0) {
                cur.guest_ok = b;
            }
            continue;
        }

        if (strcmp(key_norm, "browseable") == 0 || strcmp(key_norm, "browsable") == 0) {
            if (parse_bool(val, &b) == 0) {
                cur.browseable = b;
            }
            continue;
        }
    }

    fclose(f);

    /* No olvidar acumular la ultima seccion leida. */
    flush_share(&cur, &st);

    printf("shares=%d writable=%d guest_ok=%d browseable_no=%d\n",
           st.shares, st.writable, st.guest_ok, st.browseable_no);

    if (argc == 1) {
        return (st.shares == 3 && st.writable == 2 && st.guest_ok == 1 && st.browseable_no == 1)
                   ? EXIT_SUCCESS
                   : EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
