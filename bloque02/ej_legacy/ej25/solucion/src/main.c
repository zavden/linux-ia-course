/*
 * Ejercicio 2.5 — Enlaces Duros, Simbólicos e Inodos (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * La revelación de que los archivos EN SÍ MISMO no tienen nombre.
 * Un "directorio" es solo un post-it que ata un string arbitrario (el nombre)
 * al ID físico real del disco duro (el Inodo). 
 * Borrar con `rm` hace Under-link (unlink), decrementando las referencias. 
 * El disco solo borra los datos si las referencias llegan a 0.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // link, symlink, unlink, readlink
#include <fcntl.h>    // open
#include <sys/stat.h> // stat, lstat
#include <string.h>

void print_inode_info(const char *path, const char *label) {
    struct stat st;
    if (lstat(path, &st) == 0) {
        // %lu: unsigned long 
        printf(">> [%-10s] Inodo: %lu | Vínculos (Links) al inodo: %lu\n", 
               label, (unsigned long)st.st_ino, (unsigned long)st.st_nlink);
    } else {
        perror(path);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <base> <hardlink> <symlink>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *f_base = argv[1];
    const char *f_hard = argv[2];
    const char *f_sym  = argv[3];

    // Limpieza agresiva por si la ejecución anterior ensució el entorno
    unlink(f_base); unlink(f_hard); unlink(f_sym);

    // 1. CREACIÓN BASE
    int fd = open(f_base, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) return EXIT_FAILURE;
    write(fd, "Datos Valiosos Secretos\n", 24);
    close(fd);

    printf("1. Archivo Base Creado: %s\n", f_base);
    print_inode_info(f_base, "Base");

    // 2. MAGIA: HARD LINK
    // No estamos clonando datos. Solo creamos un alias en el directorio actual.
    if (link(f_base, f_hard) == 0) {
        printf("\n2. Hard Link Creado: %s\n", f_hard);
        print_inode_info(f_base, "Base Update"); // Ahora tiene N=2 links!
        print_inode_info(f_hard, "HardLink");    // Observa que tiene el MISMO INODO
    }

    // 3. SYM-LINK
    // Es un archivo DIFERENTE (inodo distinto) cuyo contenido String es "f_base"
    if (symlink(f_base, f_sym) == 0) {
        printf("\n3. SymLink Creado: %s\n", f_sym);
        print_inode_info(f_sym, "SymLink");
    }

    // 4. "BORRADO" DEL BASE MENTALMENTE
    printf("\n4. Eliminando archivo original con unlink()...\n");
    unlink(f_base);

    // 5. EVALUACIÓN FINAL
    // El Hard Link conserva la data y ahora su número de vinculos debe haber caído a 1.
    printf("\n5. Estado actual de los Enlaces Restantes:\n");
    print_inode_info(f_hard, "HardLink");

    // Intentamos leer del Symlink
    char sym_target[256] = {0};
    // readlink no pone el \0 final, lo tenemos que poner a mano!
    ssize_t s = readlink(f_sym, sym_target, sizeof(sym_target) - 1);
    if (s > 0) {
        sym_target[s] = '\0';
        printf(">> [SymLink   ] Sigue apuntando ciegamente a '%s'\n", sym_target);
    }

    // Intento de abrir el symlink (El SO lo resolverá hacia f_base, que fue un-linkeado, así que fallará)
    int bad_fd = open(f_sym, O_RDONLY);
    if (bad_fd < 0) {
        printf(">> [SymLink   ] Intentar abrirlo da error: ");
        fflush(stdout); // Fuerzo flush antes de perror
        perror("");
    }

    // Leemos el hard link para demostrar que los datos sobrevivieron
    int good_fd = open(f_hard, O_RDONLY);
    if (good_fd >= 0) {
        char buf[128] = {0};
        read(good_fd, buf, 24);
        printf(">> [HardLink  ] Datos Rescatados intactos: %s", buf);
        close(good_fd);
    }

    return EXIT_SUCCESS;
}
