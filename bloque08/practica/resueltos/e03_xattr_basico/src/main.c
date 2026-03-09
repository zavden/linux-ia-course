#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__APPLE__)
#include <sys/xattr.h>
#define ATTR_NAME "com.b08.note"
#else
#include <sys/xattr.h>
#define ATTR_NAME "user.b08.note"
#endif

/* Wrappers cross-platform para API xattr (Linux vs macOS difieren en firma). */
static int xset(const char *path, const char *name, const void *val, size_t len) {
#if defined(__APPLE__)
    return setxattr(path, name, val, len, 0, 0);
#else
    return setxattr(path, name, val, len, 0);
#endif
}

static ssize_t xget(const char *path, const char *name, void *buf, size_t len) {
#if defined(__APPLE__)
    return getxattr(path, name, buf, len, 0, 0);
#else
    return getxattr(path, name, buf, len);
#endif
}

static ssize_t xlist(const char *path, char *buf, size_t len) {
#if defined(__APPLE__)
    return listxattr(path, buf, len, 0);
#else
    return listxattr(path, buf, len);
#endif
}

static int xremove(const char *path, const char *name) {
#if defined(__APPLE__)
    return removexattr(path, name, 0);
#else
    return removexattr(path, name);
#endif
}

static int fallback_sidecar(const char *path, const char *value, char *out, size_t out_sz) {
    char sidecar[512];
    snprintf(sidecar, sizeof(sidecar), "%s.sidecar", path);

    FILE *f = fopen(sidecar, "w");
    if (!f) {
        return -1;
    }
    fprintf(f, "%s\n", value);
    fclose(f);

    f = fopen(sidecar, "r");
    if (!f) {
        return -1;
    }
    if (!fgets(out, (int)out_sz, f)) {
        fclose(f);
        return -1;
    }
    fclose(f);

    size_t n = strlen(out);
    if (n > 0 && out[n - 1] == '\n') {
        out[n - 1] = '\0';
    }

    unlink(sidecar);
    return 0;
}

int main(void) {
    /* Archivo temporal aislado para no tocar datos del usuario. */
    char tpl[] = "/tmp/b08_e03_XXXXXX";
    int fd = mkstemp(tpl);
    if (fd == -1) {
        perror("mkstemp");
        return EXIT_FAILURE;
    }
    close(fd);

    const char *value = "hola_xattr";
    /* Buffer donde validamos lectura del atributo (o fallback). */
    char readbuf[128] = {0};

    /* mode_sidecar=1 cuando filesystem/entorno no soporta xattr real. */
    int mode_sidecar = 0;

    if (xset(tpl, ATTR_NAME, value, strlen(value)) == -1) {
        if (errno == ENOTSUP || errno == EOPNOTSUPP || errno == EPERM || errno == EACCES) {
            mode_sidecar = 1;
            /* Fallback didáctico: conserva flujo lógico set/get/remove. */
            if (fallback_sidecar(tpl, value, readbuf, sizeof(readbuf)) == -1) {
                perror("fallback_sidecar");
                unlink(tpl);
                return EXIT_FAILURE;
            }
        } else {
            perror("setxattr");
            unlink(tpl);
            return EXIT_FAILURE;
        }
    } else {
        /* Leemos valor almacenado en xattr para verificar persistencia. */
        ssize_t n = xget(tpl, ATTR_NAME, readbuf, sizeof(readbuf) - 1);
        if (n < 0) {
            perror("getxattr");
            unlink(tpl);
            return EXIT_FAILURE;
        }
        readbuf[n] = '\0';

        /* listxattr para comprobar que el atributo aparece en metadata. */
        char names[512] = {0};
        ssize_t ln = xlist(tpl, names, sizeof(names));
        if (ln < 0) {
            perror("listxattr");
            unlink(tpl);
            return EXIT_FAILURE;
        }

        /* remove para limpiar el estado del archivo temporal. */
        if (xremove(tpl, ATTR_NAME) == -1) {
            perror("removexattr");
            unlink(tpl);
            return EXIT_FAILURE;
        }
    }

    /* Mostramos modo usado para entender comportamiento por plataforma/FS. */
    printf("mode=%s value=%s\n", mode_sidecar ? "sidecar" : "xattr", readbuf);

    unlink(tpl);
    return (strcmp(readbuf, value) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
