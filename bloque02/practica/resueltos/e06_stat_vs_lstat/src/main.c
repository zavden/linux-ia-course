#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

/* Traduce bits de tipo a string legible */
static const char *type_name(mode_t mode) {
    if (S_ISREG(mode)) return "regular";
    if (S_ISDIR(mode)) return "dir";
    if (S_ISLNK(mode)) return "symlink";
    return "other";
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <ruta>\n", argv[0]);
        return EXIT_FAILURE;
    }

    struct stat st_follow;
    struct stat st_nofollow;

    if (stat(argv[1], &st_follow) == -1) {
        perror("stat");
        return EXIT_FAILURE;
    }

    if (lstat(argv[1], &st_nofollow) == -1) {
        perror("lstat");
        return EXIT_FAILURE;
    }

    printf("type_stat=%s ino_stat=%llu nlink_stat=%llu\n",
           type_name(st_follow.st_mode),
           (unsigned long long)st_follow.st_ino,
           (unsigned long long)st_follow.st_nlink);

    printf("type_lstat=%s ino_lstat=%llu nlink_lstat=%llu\n",
           type_name(st_nofollow.st_mode),
           (unsigned long long)st_nofollow.st_ino,
           (unsigned long long)st_nofollow.st_nlink);

    return EXIT_SUCCESS;
}
