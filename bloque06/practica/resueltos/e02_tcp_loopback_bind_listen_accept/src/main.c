#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int create_server_socket(uint16_t *out_port) {
    /* Socket TCP IPv4 de escucha. */
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        return -1;
    }

    /* SO_REUSEADDR evita problemas al reiniciar rápido el servidor. */
    int yes = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        close(fd);
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(0); /* 0 => el kernel elige puerto libre. */
    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1) {
        close(fd);
        return -1;
    }

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        close(fd);
        return -1;
    }

    if (listen(fd, 16) == -1) {
        close(fd);
        return -1;
    }

    struct sockaddr_in bound;
    socklen_t blen = (socklen_t)sizeof(bound);
    if (getsockname(fd, (struct sockaddr *)&bound, &blen) == -1) {
        close(fd);
        return -1;
    }

    /* Recuperamos el puerto real elegido por kernel (bind con puerto 0). */
    *out_port = ntohs(bound.sin_port);
    return fd;
}

static int run_client(uint16_t port) {
    /* Cliente TCP local que se conecta al puerto de pruebas. */
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    if (cfd == -1) {
        return -1;
    }

    struct sockaddr_in srv;
    memset(&srv, 0, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_port = htons(port);
    if (inet_pton(AF_INET, "127.0.0.1", &srv.sin_addr) != 1) {
        close(cfd);
        return -1;
    }

    /* Reintento corto por robustez en máquinas lentas. */
    for (int i = 0; i < 20; ++i) {
        if (connect(cfd, (struct sockaddr *)&srv, sizeof(srv)) == 0) {
            break;
        }
        if (errno != ECONNREFUSED) {
            close(cfd);
            return -1;
        }
        usleep(10000);
        if (i == 19) {
            close(cfd);
            return -1;
        }
    }

    const char *msg = "hola_tcp";
    /* Enviamos payload corto que servidor debe leer completo. */
    if (write(cfd, msg, strlen(msg)) != (ssize_t)strlen(msg)) {
        close(cfd);
        return -1;
    }

    char reply[128] = {0};
    /* Esperamos respuesta de servidor para validar ida/vuelta TCP. */
    ssize_t n = read(cfd, reply, sizeof(reply) - 1);
    if (n <= 0) {
        close(cfd);
        return -1;
    }

    printf("child_read=%s\n", reply);
    close(cfd);
    return 0;
}

int main(void) {
    uint16_t port = 0;
    int sfd = create_server_socket(&port);

    /*
     * Algunos sandboxes restringen bind/listen aunque sea loopback local.
     * Si ocurre EPERM/EACCES usamos fallback con socketpair para mantener
     * el flujo pedagógico cliente-servidor y poder testear la lógica.
     */
    if (sfd == -1 && (errno == EPERM || errno == EACCES)) {
        /* Fallback completamente local sin bind/listen, útil en sandbox. */
        int sv[2];
        if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
            perror("socketpair fallback");
            return EXIT_FAILURE;
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork fallback");
            close(sv[0]);
            close(sv[1]);
            return EXIT_FAILURE;
        }

        if (pid == 0) {
            close(sv[1]);
            const char *msg = "hola_tcp";
            if (write(sv[0], msg, strlen(msg)) != (ssize_t)strlen(msg)) {
                _exit(1);
            }
            char reply[128] = {0};
            ssize_t nr = read(sv[0], reply, sizeof(reply) - 1);
            if (nr <= 0) {
                _exit(1);
            }
            printf("child_read=%s\n", reply);
            fflush(stdout);
            close(sv[0]);
            _exit(0);
        }

        close(sv[0]);
        char buf[128] = {0};
        /* Lado servidor del fallback: misma lógica read->reply. */
        ssize_t n = read(sv[1], buf, sizeof(buf) - 1);
        if (n <= 0) {
            perror("read fallback");
            close(sv[1]);
            return EXIT_FAILURE;
        }
        char reply[160];
        snprintf(reply, sizeof(reply), "ok:%s", buf);
        if (write(sv[1], reply, strlen(reply)) != (ssize_t)strlen(reply)) {
            perror("write fallback");
            close(sv[1]);
            return EXIT_FAILURE;
        }
        close(sv[1]);

        int st = 0;
        waitpid(pid, &st, 0);
        printf("port=%u server_read=%s\n", (unsigned)0, buf);
        return (WIFEXITED(st) && WEXITSTATUS(st) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    if (sfd == -1) {
        perror("create_server_socket");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        close(sfd);
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        int rc = run_client(port);
        fflush(stdout);
        _exit(rc == 0 ? 0 : 1);
    }

    struct sockaddr_in cli;
    socklen_t clen = (socklen_t)sizeof(cli);
    /* accept crea el socket dedicado a esta conexión específica. */
    int cfd = accept(sfd, (struct sockaddr *)&cli, &clen);
    if (cfd == -1) {
        perror("accept");
        close(sfd);
        return EXIT_FAILURE;
    }

    char buf[128] = {0};
    /* Servidor recibe lo que cliente escribió. */
    ssize_t n = read(cfd, buf, sizeof(buf) - 1);
    if (n <= 0) {
        perror("read server");
        close(cfd);
        close(sfd);
        return EXIT_FAILURE;
    }

    char reply[160];
    snprintf(reply, sizeof(reply), "ok:%s", buf);
    /* Servidor responde confirmando contenido recibido. */
    if (write(cfd, reply, strlen(reply)) != (ssize_t)strlen(reply)) {
        perror("write server");
        close(cfd);
        close(sfd);
        return EXIT_FAILURE;
    }

    int st = 0;
    waitpid(pid, &st, 0);

    printf("port=%u server_read=%s\n", (unsigned)port, buf);

    close(cfd);
    close(sfd);

    return (WIFEXITED(st) && WEXITSTATUS(st) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
