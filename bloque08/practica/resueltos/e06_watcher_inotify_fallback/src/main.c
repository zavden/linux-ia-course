#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#if defined(__linux__)
#include <sys/inotify.h>
#endif

static int create_marker_file(const char *path) {
    int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0600);
    if (fd == -1) {
        return -1;
    }
    const char *msg = "ok";
    if (write(fd, msg, 2) != 2) {
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

/* Fallback portable: espera por existencia de archivo por polling corto. */
static int watch_with_polling(const char *target, int timeout_ms) {
    int elapsed = 0;
    while (elapsed < timeout_ms) {
        if (access(target, F_OK) == 0) {
            return 1;
        }
        usleep(10000);
        elapsed += 10;
    }
    return 0;
}

int main(void) {
    /* Directorio temporal para no depender de rutas del host. */
    char dir_tpl[] = "/tmp/b08_e06_dir_XXXXXX";
    int tfd = mkstemp(dir_tpl);
    if (tfd == -1) {
        perror("mkstemp");
        return EXIT_FAILURE;
    }
    close(tfd);
    unlink(dir_tpl);
    if (mkdir(dir_tpl, 0700) == -1) {
        perror("mkdir tempdir");
        return EXIT_FAILURE;
    }
    const char *dir = dir_tpl;

    char marker[512];
    snprintf(marker, sizeof(marker), "%s/newfile.txt", dir);

#if defined(__linux__)
    /* Camino principal Linux: eventos reales de kernel vía inotify. */
    int inofd = inotify_init1(IN_NONBLOCK);
    if (inofd != -1) {
        int wd = inotify_add_watch(inofd, dir, IN_CREATE);
        if (wd == -1) {
            perror("inotify_add_watch");
            close(inofd);
            rmdir(dir);
            return EXIT_FAILURE;
        }

        if (create_marker_file(marker) == -1) {
            perror("create_marker_file");
            close(inofd);
            rmdir(dir);
            return EXIT_FAILURE;
        }

        int detected = 0;
        char buf[1024];
        /* Loop corto de espera no bloqueante para mantener test rápido. */
        for (int i = 0; i < 50; ++i) {
            ssize_t n = read(inofd, buf, sizeof(buf));
            if (n > 0) {
                detected = 1;
                break;
            }
            if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                usleep(10000);
                continue;
            }
            break;
        }

        inotify_rm_watch(inofd, wd);
        close(inofd);
        unlink(marker);
        rmdir(dir);

        /* Identificamos método usado para transparencia del ejercicio. */
        printf("method=inotify detected=%d\n", detected);
        return detected ? EXIT_SUCCESS : EXIT_FAILURE;
    }
#endif

    /* Ruta portable cuando inotify no existe o no está disponible. */
    if (create_marker_file(marker) == -1) {
        perror("create_marker_file fallback");
        rmdir(dir);
        return EXIT_FAILURE;
    }

    int detected = watch_with_polling(marker, 500);

    unlink(marker);
    rmdir(dir);

    printf("method=polling detected=%d\n", detected);
    return detected ? EXIT_SUCCESS : EXIT_FAILURE;
}
