#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int have_kptr;
    int have_dmesg;
    int have_unpriv_bpf;
    int have_symlink;
    int have_rpf;
    int have_swap;

    int pass_kptr;
    int pass_dmesg;
    int pass_unpriv_bpf;
    int pass_symlink;
    int pass_rpf;
    int pass_swap;
} policy_eval_t;

static char *trim(char *s) {
    while (*s != '\0' && isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;

    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end-- = '\0';
    }
    return s;
}

static int parse_long(const char *s, long *out) {
    errno = 0;
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') {
        return -1;
    }
    *out = v;
    return 0;
}

int main(int argc, char **argv) {
    const char *path = (argc == 2) ? argv[1] : "tests/data/kernel.good.conf";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<kernel_conf>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    policy_eval_t ev;
    memset(&ev, 0, sizeof(ev));

    char line[1024];
    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';

        char *comment = strchr(line, '#');
        if (comment) {
            *comment = '\0';
        }

        char *p = trim(line);
        if (*p == '\0') {
            continue;
        }

        char *eq = strchr(p, '=');
        if (eq == NULL) {
            continue;
        }

        *eq = '\0';
        char *key = trim(p);
        char *val = trim(eq + 1);

        long v = 0;
        if (parse_long(val, &v) != 0) {
            continue;
        }

        if (strcmp(key, "kernel.kptr_restrict") == 0) {
            ev.have_kptr = 1;
            ev.pass_kptr = (v == 1) ? 1 : 0;
            continue;
        }

        if (strcmp(key, "kernel.dmesg_restrict") == 0) {
            ev.have_dmesg = 1;
            ev.pass_dmesg = (v == 1) ? 1 : 0;
            continue;
        }

        if (strcmp(key, "kernel.unprivileged_bpf_disabled") == 0) {
            ev.have_unpriv_bpf = 1;
            ev.pass_unpriv_bpf = (v == 1) ? 1 : 0;
            continue;
        }

        if (strcmp(key, "fs.protected_symlinks") == 0) {
            ev.have_symlink = 1;
            ev.pass_symlink = (v == 1) ? 1 : 0;
            continue;
        }

        if (strcmp(key, "net.ipv4.conf.all.rp_filter") == 0) {
            ev.have_rpf = 1;
            ev.pass_rpf = (v == 1) ? 1 : 0;
            continue;
        }

        if (strcmp(key, "vm.swappiness") == 0) {
            ev.have_swap = 1;
            ev.pass_swap = (v >= 0 && v <= 60) ? 1 : 0;
            continue;
        }
    }

    fclose(f);

    int checks = 6;
    int pass = ev.pass_kptr + ev.pass_dmesg + ev.pass_unpriv_bpf + ev.pass_symlink + ev.pass_rpf + ev.pass_swap;

    /* Falta de clave requerida se considera fallo de política. */
    if (!ev.have_kptr) pass--;
    if (!ev.have_dmesg) pass--;
    if (!ev.have_unpriv_bpf) pass--;
    if (!ev.have_symlink) pass--;
    if (!ev.have_rpf) pass--;
    if (!ev.have_swap) pass--;

    if (pass < 0) {
        pass = 0;
    }

    int fail = checks - pass;

    const char *status = "OK";
    if (fail >= 2) {
        status = "CRIT";
    } else if (fail == 1) {
        status = "WARN";
    }

    printf("checks=%d pass=%d fail=%d status=%s\n", checks, pass, fail, status);

    return (strcmp(status, "OK") == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
