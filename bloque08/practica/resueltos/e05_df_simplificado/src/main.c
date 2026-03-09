#define _POSIX_C_SOURCE 200809L
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/statvfs.h>

static uint64_t kib(uint64_t blocks, uint64_t block_size) {
    return (blocks * block_size) / 1024ULL;
}

static int print_fs_line(const char *path) {
    /* statvfs sobre cada ruta para obtener métricas del FS asociado. */
    struct statvfs st;
    if (statvfs(path, &st) != 0) {
        return -1;
    }

    uint64_t total = kib((uint64_t)st.f_blocks, (uint64_t)st.f_frsize);
    uint64_t avail = kib((uint64_t)st.f_bavail, (uint64_t)st.f_frsize);
    /* Cálculo simple de usado y porcentaje por ruta. */
    uint64_t used = (total >= avail) ? (total - avail) : 0;

    double used_pct = (total > 0) ? (100.0 * (double)used / (double)total) : 0.0;

    printf("fs path=%s total=%" PRIu64 " avail=%" PRIu64 " used_pct=%.2f\n", path, total, avail, used_pct);
    return 0;
}

int main(int argc, char **argv) {
    /* Si no hay args, usamos dos rutas comunes para demo. */
    const char *defaults[] = {".", "/tmp"};

    int ok = 0;
    int fail = 0;

    if (argc <= 1) {
        /* Ruta rápida demo: dos paths por defecto. */
        for (size_t i = 0; i < sizeof(defaults) / sizeof(defaults[0]); ++i) {
            if (print_fs_line(defaults[i]) == 0) {
                ok++;
            } else {
                fail++;
            }
        }
    } else {
        /* Modo explícito: procesa exactamente rutas entregadas por CLI. */
        for (int i = 1; i < argc; ++i) {
            if (print_fs_line(argv[i]) == 0) {
                ok++;
            } else {
                fail++;
            }
        }
    }

    /* Métrica final para uso en scripts: cuántas rutas se pudieron medir. */
    printf("ok=%d fail=%d\n", ok, fail);
    return (ok > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
