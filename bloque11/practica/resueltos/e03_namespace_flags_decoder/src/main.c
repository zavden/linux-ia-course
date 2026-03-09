#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Valores de flags CLONE_NEW* (ABI Linux).
 * Definimos constantes locales para no depender de headers específicos.
 */
#define CLONE_NEWNS      0x00020000U
#define CLONE_NEWCGROUP  0x02000000U
#define CLONE_NEWUTS     0x04000000U
#define CLONE_NEWIPC     0x08000000U
#define CLONE_NEWUSER    0x10000000U
#define CLONE_NEWPID     0x20000000U
#define CLONE_NEWNET     0x40000000U

static int bit(uint32_t mask, uint32_t flag) {
    return (mask & flag) ? 1 : 0;
}

int main(int argc, char **argv) {
    const char *mask_text = (argc == 2) ? argv[1] : "0x64020000";

    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Uso: %s [<mask_hex>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    errno = 0;
    char *end = NULL;
    unsigned long parsed = strtoul(mask_text, &end, 0);
    if (errno != 0 || end == mask_text || *end != '\0') {
        fprintf(stderr, "Mask invalida\n");
        return EXIT_FAILURE;
    }

    uint32_t mask = (uint32_t)parsed;

    int ns_mnt = bit(mask, CLONE_NEWNS);
    int ns_uts = bit(mask, CLONE_NEWUTS);
    int ns_ipc = bit(mask, CLONE_NEWIPC);
    int ns_user = bit(mask, CLONE_NEWUSER);
    int ns_pid = bit(mask, CLONE_NEWPID);
    int ns_net = bit(mask, CLONE_NEWNET);
    int ns_cgroup = bit(mask, CLONE_NEWCGROUP);

    int count = ns_mnt + ns_uts + ns_ipc + ns_user + ns_pid + ns_net + ns_cgroup;

    printf("mnt=%d uts=%d ipc=%d user=%d pid=%d net=%d cgroup=%d count=%d\n",
           ns_mnt, ns_uts, ns_ipc, ns_user, ns_pid, ns_net, ns_cgroup, count);

    if (argc == 1) {
        return (ns_mnt == 1 && ns_uts == 1 && ns_pid == 1 && ns_net == 1 &&
                ns_ipc == 0 && ns_user == 0 && ns_cgroup == 0 && count == 4)
                   ? EXIT_SUCCESS
                   : EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
