#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int tls_required;
    int mtls_internal;
    int secrets_rotation_days;
    int audit_logs;
    int backup_enabled;
    int health_interval_sec;

    int have_tls_required;
    int have_mtls_internal;
    int have_secrets_rotation_days;
    int have_audit_logs;
    int have_backup_enabled;
    int have_health_interval_sec;
} policy_t;

static char *trim(char *s) {
    while (*s != '\0' && isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)end[-0])) *end-- = '\0';
    return s;
}

static int parse_int(const char *s, int *out) {
    errno = 0;
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') return 0;
    *out = (int)v;
    return 1;
}

int main(int argc, char **argv) {
    const char *path = (argc == 2) ? argv[1] : "tests/data/policy.good.conf";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<policy_file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    policy_t p;
    memset(&p, 0, sizeof(p));

    char line[512];
    while (fgets(line, sizeof(line), f) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0' || line[0] == '#') continue;

        char *eq = strchr(line, '=');
        if (!eq) continue;

        *eq = '\0';
        char *k = trim(line);
        char *v = trim(eq + 1);

        int x = 0;
        if (!parse_int(v, &x)) continue;

        if (strcmp(k, "tls_required") == 0) {
            p.have_tls_required = 1; p.tls_required = x; continue;
        }
        if (strcmp(k, "mtls_internal") == 0) {
            p.have_mtls_internal = 1; p.mtls_internal = x; continue;
        }
        if (strcmp(k, "secrets_rotation_days") == 0) {
            p.have_secrets_rotation_days = 1; p.secrets_rotation_days = x; continue;
        }
        if (strcmp(k, "audit_logs") == 0) {
            p.have_audit_logs = 1; p.audit_logs = x; continue;
        }
        if (strcmp(k, "backup_enabled") == 0) {
            p.have_backup_enabled = 1; p.backup_enabled = x; continue;
        }
        if (strcmp(k, "health_interval_sec") == 0) {
            p.have_health_interval_sec = 1; p.health_interval_sec = x; continue;
        }
    }

    fclose(f);

    int checks = 6;
    int pass = 0;

    /* Reglas de politica: combinan seguridad + operacion. */
    if (p.have_tls_required && p.tls_required == 1) pass++;
    if (p.have_mtls_internal && p.mtls_internal == 1) pass++;
    if (p.have_secrets_rotation_days && p.secrets_rotation_days > 0 && p.secrets_rotation_days <= 90) pass++;
    if (p.have_audit_logs && p.audit_logs == 1) pass++;
    if (p.have_backup_enabled && p.backup_enabled == 1) pass++;
    if (p.have_health_interval_sec && p.health_interval_sec >= 5 && p.health_interval_sec <= 120) pass++;

    int fail = checks - pass;

    const char *status = "OK";
    if (fail >= 3) status = "CRIT";
    else if (fail >= 1) status = "WARN";

    printf("checks=%d pass=%d fail=%d status=%s\n", checks, pass, fail, status);

    return (strcmp(status, "OK") == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
