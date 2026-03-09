#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int write_text_file(const char *path, const char *text) {
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) return -1;
    size_t len = strlen(text);
    ssize_t w = write(fd, text, len);
    close(fd);
    return (w == (ssize_t)len) ? 0 : -1;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <base> <hard> <sym>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *base = argv[1];
    const char *hard = argv[2];
    const char *sym = argv[3];

    unlink(base);
    unlink(hard);
    unlink(sym);

    if (write_text_file(base, "Hola Mundo\n") == -1) {
        perror("write_text_file");
        return EXIT_FAILURE;
    }

    if (link(base, hard) == -1) {
        perror("link");
        return EXIT_FAILURE;
    }

    if (symlink(base, sym) == -1) {
        perror("symlink");
        return EXIT_FAILURE;
    }

    struct stat st_base, st_hard, st_sym;
    if (stat(base, &st_base) == -1 || stat(hard, &st_hard) == -1 || lstat(sym, &st_sym) == -1) {
        perror("stat/lstat");
        return EXIT_FAILURE;
    }

    printf("ino_base=%llu ino_hard=%llu ino_sym=%llu\n",
           (unsigned long long)st_base.st_ino,
           (unsigned long long)st_hard.st_ino,
           (unsigned long long)st_sym.st_ino);

    if (unlink(base) == -1) {
        perror("unlink base");
        return EXIT_FAILURE;
    }

    int fd_h = open(hard, O_RDONLY);
    if (fd_h == -1) {
        perror("open hard");
        return EXIT_FAILURE;
    }

    char buf[64] = {0};
    ssize_t r = read(fd_h, buf, sizeof(buf) - 1);
    close(fd_h);

    if (r == -1) {
        perror("read hard");
        return EXIT_FAILURE;
    }

    printf("hard_data=%s", buf);

    int fd_s = open(sym, O_RDONLY);
    if (fd_s == -1) {
        printf("symlink_open_error=%s\n", strerror(errno));
    } else {
        close(fd_s);
        printf("symlink_open_error=none\n");
    }

    return EXIT_SUCCESS;
}
