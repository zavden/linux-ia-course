#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#define SHM_NAME "/b04_e05_demo"
#define SHM_SIZE 1024

/*
 * Cuando no hay MAP_ANONYMOUS/MAP_ANON disponible en headers,
 * mapeamos sobre un archivo temporal en /tmp.
 */
static int open_tmp_backing(size_t n) {
    char tpl[] = "/tmp/b04_e05_XXXXXX";
    int fd = mkstemp(tpl);
    if (fd == -1) {
        return -1;
    }

    unlink(tpl);

    if (ftruncate(fd, (off_t)n) == -1) {
        int saved = errno;
        close(fd);
        errno = saved;
        return -1;
    }

    return fd;
}

/*
 * Crea una región compartida "clásica" con POSIX SHM:
 * 1) shm_open crea/abre el objeto con nombre global.
 * 2) ftruncate fija su tamaño.
 * 3) mmap lo mapea en el espacio del proceso.
 *
 * Si algo falla devolvemos -1 con errno preservado para que el caller
 * pueda decidir si aborta o aplica fallback.
 */
static int map_posix_shm(char **out_mem, int *out_need_unlink) {
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
    if (fd == -1) {
        return -1;
    }

    if (ftruncate(fd, SHM_SIZE) == -1) {
        int saved = errno;
        close(fd);
        shm_unlink(SHM_NAME);
        errno = saved;
        return -1;
    }

    char *mem = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mem == MAP_FAILED) {
        int saved = errno;
        close(fd);
        shm_unlink(SHM_NAME);
        errno = saved;
        return -1;
    }

    close(fd);
    *out_mem = mem;
    *out_need_unlink = 1;
    return 0;
}

/*
 * Fallback para entornos restringidos (algunos sandboxes bloquean shm_open):
 * usamos mmap MAP_SHARED anónimo (o archivo temporal si no existe la bandera).
 *
 * Pedagógicamente sigue mostrando lo importante del ejercicio:
 * padre e hijo comparten la misma región tras fork().
 */
static int map_shared_fallback(char **out_mem) {
    int fd = -1;
    int flags = MAP_SHARED;

#if defined(MAP_ANONYMOUS)
    flags |= MAP_ANONYMOUS;
#elif defined(MAP_ANON)
    flags |= MAP_ANON;
#else
    fd = open_tmp_backing(SHM_SIZE);
    if (fd == -1) {
        return -1;
    }
#endif

    char *mem = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, flags, fd, 0);
    if (fd != -1) {
        close(fd);
    }
    if (mem == MAP_FAILED) {
        return -1;
    }

    *out_mem = mem;
    return 0;
}

int main(void) {
    char *mem = NULL;
    int need_unlink = 0;

    /*
     * Primero intentamos la vía "oficial" del tema (POSIX SHM).
     * Si el entorno lo impide, caemos a fallback compartido para que el
     * ejercicio siga siendo ejecutable y comprobable.
     */
    if (map_posix_shm(&mem, &need_unlink) == -1) {
        int saved = errno;
        fprintf(stderr,
                "aviso: shm_open no disponible (%s), usando mmap compartido\n",
                strerror(saved));
        if (map_shared_fallback(&mem) == -1) {
            perror("map_shared_fallback");
            return EXIT_FAILURE;
        }
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        munmap(mem, SHM_SIZE);
        if (need_unlink) {
            shm_unlink(SHM_NAME);
        }
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        /*
         * El hijo espera un instante para asegurar que el padre escribió
         * primero; esto evita carreras al validar salida en tests simples.
         */
        sleep(1);
        printf("hijo_lee=%s\n", mem);
        fflush(stdout);

        /* El hijo sobrescribe el mensaje para devolver una "respuesta". */
        snprintf(mem, SHM_SIZE, "respuesta_hijo");
        _exit(0);
    }

    /* Mensaje inicial escrito por el padre, visible para el hijo. */
    snprintf(mem, SHM_SIZE, "mensaje_padre");

    int st = 0;
    waitpid(pid, &st, 0);

    /* Tras waitpid ya vemos en padre lo escrito por el hijo. */
    printf("padre_lee=%s\n", mem);

    munmap(mem, SHM_SIZE);
    if (need_unlink) {
        shm_unlink(SHM_NAME);
    }

    return EXIT_SUCCESS;
}
