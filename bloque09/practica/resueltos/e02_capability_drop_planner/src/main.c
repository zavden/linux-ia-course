#define _POSIX_C_SOURCE 200809L
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

/* Capabilities de ejemplo usadas para política mínima. */
enum {
    CAP_CHOWN = 0,
    CAP_DAC_OVERRIDE = 1,
    CAP_NET_BIND_SERVICE = 10,
    CAP_NET_RAW = 13,
};

/* Cuenta bits en máscara (popcount manual portable). */
static int popcount64(uint64_t x) {
    int n = 0;
    while (x != 0) {
        n += (int)(x & 1ULL);
        x >>= 1;
    }
    return n;
}

int main(void) {
    /*
     * current: CHOWN + DAC_OVERRIDE + NET_BIND_SERVICE + NET_RAW
     * binary: bits 0,1,10,13
     */
    uint64_t current = (1ULL << CAP_CHOWN) |
                       (1ULL << CAP_DAC_OVERRIDE) |
                       (1ULL << CAP_NET_BIND_SERVICE) |
                       (1ULL << CAP_NET_RAW);

    /* Política: solo conservar CAP_NET_BIND_SERVICE. */
    uint64_t required = (1ULL << CAP_NET_BIND_SERVICE);

    uint64_t keep = current & required;
    uint64_t drop = current & ~required;

    int keep_n = popcount64(keep);
    int drop_n = popcount64(drop);

    printf("current=0x%llx keep=0x%llx drop=0x%llx keep_n=%d drop_n=%d\n",
           (unsigned long long)current,
           (unsigned long long)keep,
           (unsigned long long)drop,
           keep_n,
           drop_n);

    return (keep_n == 1 && drop_n == 3) ? EXIT_SUCCESS : EXIT_FAILURE;
}
