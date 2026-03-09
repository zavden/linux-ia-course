#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int ns_total;
    int unique_ids;
    int has_user;
    int has_time;
} ns_stats_t;

static int seen_id(const unsigned long long *ids, int n, unsigned long long id) {
    for (int i = 0; i < n; ++i) {
        if (ids[i] == id) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    const char *path = (argc == 2) ? argv[1] : "tests/data/pidns.sample";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<pid_ns_file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    ns_stats_t st = {0, 0, 0, 0};
    unsigned long long ids[64];
    int n_ids = 0;

    char line[256];
    while (fgets(line, sizeof(line), f) != NULL) {
        char name[64] = {0};
        unsigned long long inode = 0ULL;

        /* Formato simplificado: name:[inode] */
        int n = sscanf(line, "%63[^:]:[%llu]", name, &inode);
        if (n != 2) {
            continue;
        }

        st.ns_total++;

        if (!seen_id(ids, n_ids, inode) && n_ids < (int)(sizeof(ids) / sizeof(ids[0]))) {
            ids[n_ids++] = inode;
        }

        if (strcmp(name, "user") == 0) {
            st.has_user = 1;
        }
        if (strcmp(name, "time") == 0) {
            st.has_time = 1;
        }
    }

    fclose(f);

    st.unique_ids = n_ids;

    printf("ns_total=%d unique_ids=%d has_user=%d has_time=%d\n",
           st.ns_total, st.unique_ids, st.has_user, st.has_time);

    if (argc == 1) {
        return (st.ns_total == 8 && st.unique_ids == 8 && st.has_user == 1 && st.has_time == 1)
                   ? EXIT_SUCCESS
                   : EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
