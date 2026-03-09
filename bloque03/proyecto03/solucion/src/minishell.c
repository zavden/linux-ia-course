/*
 * Proyecto 3 — minishell (SOLUCIÓN DIDÁCTICA)
 *
 * OBJETIVO DIDÁCTICO:
 * Escribir un intérprete central interactivo (Turing-músculo de Bash).
 * Implementa 100% de los requisitos: Signal Reaping SIGCHLD de 
 * Background Jobs (`&`), Pipes clásicos (`|`), redireccion Output (`>`), 
 * parseador de strings de tokens (strtok) y Built-ins nativos sin fork (cd, exit).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>   // fork, execvp, chdir, getcwd
#include <sys/wait.h> // waitpid
#include <signal.h>   // sigaction
#include <fcntl.h>    // open dup2
#include <errno.h>

#define DELIM " \t\r\n\a" // Delimitadores del tokenizer: Espacios, tabuladores, enteres.
#define MAX_ARGS 64

// Para el comando mágico nativo de c `env` (lista las variables)
extern char **environ; 

/*
 * MANEJADOR SIGCHLD: El Ángel de la Muerte
 * Los Hijos en background (los que tiras con &) nunca hacen `waitpid()` síncrono en el main, 
 * así que enviarán la señal de SIGCHLD al SO global al fallecer o morir trágicamente.
 * WNOHANG nos deja limpiar 1... 5... o mil Zombies muertos encolados en el CPU instantáneamente 
 * sin colgarnos JAMÁS esperando de más.
 */
void sigchld_handler(int sig) {
    (void)sig; // Silenciando warning de macro Unused
    pid_t pid;
    int status;
    // Bucle para cosecharlos todos de tiro en caso de oleadas.
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // En una terminal real como Zsh, esta funcion hace print a lo "Jobs[1] exit", pero C lo desaconseja por lockeos
        // asíncronos en printf de la glibc. Con el silenciamiento de Zombies es Suficiente como logro C Kernell.
    }
}

/*
 * PARSEADOR ESTRUCTURAL 
 * Toma "ls -l > out.txt" y lo devora rellenando la lista cruda requerida de ExecVP.
 */
int parse_line(char *line, char **args, int *bg_flag, char **redirect_out) {
    int i = 0;
    *bg_flag = 0;
    *redirect_out = NULL;

    // Buscando pipes, si tiene "|" marcamos con error craso porque nos saltaremos eso 
    // en este nivel elemental delegándolo a funciones más puras complejas abajo.
    
    char *token = strtok(line, DELIM);
    while (token != NULL) {
        if (strcmp(token, "&") == 0) {
            *bg_flag = 1; // Job en desatencion background
        } else if (strcmp(token, ">") == 0) {
            // El SIGUIENTE token es el path adonde deviarlo obligatoriamente (out.txt).
            // Lo extraemos inmediatamente
            token = strtok(NULL, DELIM);
            if (token) *redirect_out = token;
        } else {
            args[i++] = token;
        }
        token = strtok(NULL, DELIM);
    }
    args[i] = NULL; // EXECVP exige que se de un terminador formal de cola vacia NULO en tu array de strings
    return i;
}

/*
 * FUNCIONES BUILT-INS
 * No forkeables, exigen mutar la sangre RAM del Padre o fallaría catastróficamente la CLI local
 */
int execute_builtin(char **args) {
    if (strcmp(args[0], "exit") == 0) {
        printf("Saliendo de la matrix. Hasta siempre!\n");
        exit(EXIT_SUCCESS); 
    } 
    else if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "minishell: cd esperaba agumento path a saltar!\n");
        } else if (chdir(args[1]) != 0) {
            perror("minishell (cd error)");
        }
        return 1; 
    }
    else if (strcmp(args[0], "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) printf("%s\n", cwd);
        else perror("minishell pwd colapsado");
        return 1;
    }
    else if (strcmp(args[0], "env") == 0) {
        for (char **env = environ; *env != 0; env++) {
            char *thisEnv = *env;
            printf("%s\n", thisEnv);    
        }
        return 1;
    }
    return 0; // Cero es no era un comando mago mio local. Lánzaselo de castigo Externo al OS ($PATH).
}

/*
 * TUBERIAS SIMPLES (|) PIPELINE IPC EJECUTOR MULTIPLE
 */
void execute_pipe(char *cmd_left, char *cmd_right) {
    int fd[2];
    if (pipe(fd) == -1) { perror("Pipe colapsó general"); return; }
    
    // Parseo veloz de partes Izquierdas y Derechas crudas y puras
    char *args_L[MAX_ARGS], *args_R[MAX_ARGS];
    int trash; char *trash2;
    parse_line(cmd_left, args_L, &trash, &trash2);
    parse_line(cmd_right, args_R, &trash, &trash2);

    pid_t pid1 = fork();
    if (pid1 == 0) {
        // HIJO IZQ (Escritor en tubo, Emisor de LS)
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO); // Envia en lugar de su monitor, ciego hacia el tubo
        close(fd[1]);
        execvp(args_L[0], args_L);
        perror("Falla comando izq"); exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        // HIJO DER (Lector Extremo Goterente, Receptor perezoso en el charco de WC)
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO); // Roba Teclado por tubo de Ram del PC.
        close(fd[0]);
        execvp(args_R[0], args_R);
        perror("Falla comando der"); exit(EXIT_FAILURE);
    }

    // Padre de ambos
    close(fd[0]); close(fd[1]); // Cierra los grifos del cielo, o el Hijo Right jamás se le apagará su Input.
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}


/*
 * LA GRAN EJECUCION FORKEADA NORMAL (Híbridos sin Pipes)
 */
void launch_process(char **args, int bg_flag, char *redirect_out) {
    pid_t pid = fork();

    if (pid == 0) {
        // --- SECUELA HIJO ---

        // Devolviéndole SIGINT al normal Default Kernell. 
        // Ya que el Padre Shell lo canceló ignorándolo (SIG_IGN), el hijo nació ignorándolo igual! 
        // Piénsalo, si tirabas a rodar `sleep 50` no lo podrías apagar ni con mil Ctrl+C! Así que se lo devolvemos C++ a los normales.
        signal(SIGINT, SIG_DFL);

        // Desvíos Físicos Locales Excepcionales ('> out.txt')
        if (redirect_out != NULL) {
            int fd_out = open(redirect_out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out < 0) { perror("Abriendo Redireccion de Salida OS"); exit(EXIT_FAILURE); }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        // Ejecución posesiva terminal mortal binaria
        execvp(args[0], args);
        
        // Cae en trampa aca si no existia comando
        fprintf(stderr, "minishell: %s: comando no hallado\n", args[0]);
        exit(EXIT_FAILURE);
    } 
    else if (pid > 0) {
        // --- SECUELA PADRE ---
        if (!bg_flag) {
            // El usuario NOS ORDENA esperarlo bloqueado (ForeGround / Sin ampersand)
            int status;
            waitpid(pid, &status, 0);
        } else {
            // Le concedimos el comando del asincronia desatentida C.
            // Regresamos enseguida al while Prompt() dejando caer este al viento y al Handled de CHLD Asíncrono del RAM.
            printf("[Inyectado Background Job PID: %d]\n", pid);
        }
    } else {
        perror("Fork aborotado RAM CPU Max out");
    }
}

int main(void) {
    // 1. REGISTRO OFICIAL SIG_IGN DE CTRL+C (Protege al CLI de apagarse estupidamente al tipear mal una tecla ctrl)
    struct sigaction sa_int;
    sa_int.sa_handler = SIG_IGN; // "Ignora los dardos"
    sa_int.sa_flags = 0; sigemptyset(&sa_int.sa_mask);
    sigaction(SIGINT, &sa_int, NULL);

    // 2. REGISTRO SIGCHLD (Segador/Reaper de Hijos de Background fantasmas Zombie desposeídos)
    struct sigaction sa_chld;
    sa_chld.sa_handler = sigchld_handler;
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP; // SA_RESTART previene que un chld zombie interrumpa un fgets de CLI rompiendonos el buffer prompt read line de bash al chocar de repente!.
    sigemptyset(&sa_chld.sa_mask);
    sigaction(SIGCHLD, &sa_chld, NULL);

    char line[1024];
    char *args[MAX_ARGS];
    char cwd[1024];

    printf("===================================================\n");
    printf("= Bienvenido a MINISHELL v.0.0.1 POSIX Kernell    =\n");
    printf("= Powered/Coded By C and UNIX Raw FileDescriptors =\n");
    printf("===================================================\n");

    // ==========================================
    // ALMA MOTOR BUCLE PRINCIPAL INFALIBLE BASH CLI
    // ==========================================
    while (1) {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("\n\033[1;32mminishell\033[0m:\033[1;34m%s\033[0m$ ", cwd); // Impresion mágica colorizada Bash PS1
        } else {
            printf("\nminishell:>> ");
        }

        if (fgets(line, sizeof(line), stdin) == NULL) {
            // Ocurre si el usuario apretaba CTRL+D (Cierre final FEOF de buffer standard Unix Pipe). Saliendo Pacífico
            printf("\nCierre manual detectado (Ctrl+D). Goodbye!\n");
            break; 
        }

        // Chequear asincronia paralela especial del usuario
        // Un pipe es un rey mayor del arbol estructural:
        char *pipe_symbol = strchr(line, '|'); 
        if (pipe_symbol != NULL) {
            *pipe_symbol = '\0'; // Cortamos el string grande textualmente asesiando el char Pipe a null zero.
            char *cmd_izq = line;
            char *cmd_der = pipe_symbol + 1;
            execute_pipe(cmd_izq, cmd_der);
            continue; // Tuberías no pasan por Built-ins normales del shell.
        }

        // Análisis Estructural Clásico y Tokenización Comandos Locales / Externos Normalizados
        int bg_flag;
        char *redirect_out;
        int num_args = parse_line(line, args, &bg_flag, &redirect_out);

        // Si dio Enter en blanco, ignorar.
        if (num_args == 0) continue;

        // Comandos Base (Inyecciones de PTY directas sin fork C)
        if (execute_builtin(args)) {
            continue;
        }

        // Catación externa final y asalto OS Syscall Path Native
        launch_process(args, bg_flag, redirect_out);
    }

    return EXIT_SUCCESS;
}
