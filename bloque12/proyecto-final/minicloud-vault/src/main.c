#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Formato de manifest:
 * secret|scope|ttl_sec|rotatable|ref_id
 */

typedef struct {
    int valid;
    int invalid;
    int rotatable;
} vault_stats_t;

static int valid_name(const char *s) {
    if (!s || s[0] == '\0') return 0;
    size_t len = strlen(s);
    if (len < 3U || len > 48U) return 0;

    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c == '_' || c == '-' || c == '.')) return 0;
    }

    return 1;
}

static int valid_scope(const char *s) {
    return (strcmp(s, "global") == 0 || strcmp(s, "service") == 0 || strcmp(s, "tenant") == 0) ? 1 : 0;
}

static int parse_ttl(const char *s, unsigned long long *out) {
    if (!s || !out || s[0] == '\0') return 0;
    if (s[0] == '+' || s[0] == '-') return 0;

    errno = 0;
    char *end = NULL;
    unsigned long long v = strtoull(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') return 0;

    *out = v;
    return 1;
}

static int parse_rot(const char *s, int *out) {
    if (!s || !out) return 0;
    if (strcmp(s, "yes") == 0) {
        *out = 1;
        return 1;
    }
    if (strcmp(s, "no") == 0) {
        *out = 0;
        return 1;
    }
    return 0;
}

/* Busca ref de secreto por nombre para simulación de delivery. */
static int find_secret_ref(const char *path, const char *needle, char *out, size_t out_sz) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char line[512];
    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0' || line[0] == '#') continue;

        char copy[512];
        snprintf(copy, sizeof(copy), "%s", line);

        char *name = strtok(copy, "|");
        char *scope = strtok(NULL, "|");
        char *ttl = strtok(NULL, "|");
        char *rot = strtok(NULL, "|");
        char *ref = strtok(NULL, "|");

        if (!name || !scope || !ttl || !rot || !ref) continue;

        if (strcmp(name, needle) == 0) {
            snprintf(out, out_sz, "%s", ref);
            fclose(f);
            return 0;
        }
    }

    fclose(f);
    return 1;
}

int main(int argc, char **argv) {
    const char *path = "tests/data/secrets.sample";

    if (argc == 4 && strcmp(argv[1], "--get") == 0) {
        const char *name = argv[2];
        path = argv[3];

        char ref[128] = {0};
        int rc = find_secret_ref(path, name, ref, sizeof(ref));
        if (rc == 0) {
            printf("secret=%s found=1 ref=%s\n", name, ref);
            return EXIT_SUCCESS;
        }
        if (rc == 1) {
            printf("secret=%s found=0\n", name);
            return EXIT_FAILURE;
        }

        perror("find_secret_ref");
        return EXIT_FAILURE;
    }

    if (argc == 2) {
        path = argv[1];
    } else if (argc != 1) {
        fprintf(stderr, "Uso: %s [<manifest>] | --get <secret> <manifest>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    vault_stats_t st = {0, 0, 0};

    char line[512];
    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0' || line[0] == '#') continue;

        char copy[512];
        snprintf(copy, sizeof(copy), "%s", line);

        char *name = strtok(copy, "|");
        char *scope = strtok(NULL, "|");
        char *ttl_s = strtok(NULL, "|");
        char *rot_s = strtok(NULL, "|");
        char *ref = strtok(NULL, "|");
        char *extra = strtok(NULL, "|");

        if (!name || !scope || !ttl_s || !rot_s || !ref || extra) {
            st.invalid++;
            continue;
        }

        unsigned long long ttl = 0;
        int rot = 0;

        int ok = valid_name(name) && valid_scope(scope) && parse_ttl(ttl_s, &ttl) &&
                 parse_rot(rot_s, &rot) && valid_name(ref) && ttl >= 60ULL && ttl <= 31536000ULL;

        if (!ok) {
            st.invalid++;
            continue;
        }

        st.valid++;
        if (rot) st.rotatable++;
    }

    fclose(f);

    int ready = (st.invalid == 0 && st.valid >= 3) ? 1 : 0;
    printf("valid=%d invalid=%d rotatable=%d ready=%d\n", st.valid, st.invalid, st.rotatable, ready);

    return ready ? EXIT_SUCCESS : EXIT_FAILURE;
}
