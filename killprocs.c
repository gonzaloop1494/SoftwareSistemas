// Programa que recibe al menos un argumento
// Lor argumentos serán PIDs de procesos.
// El programa debe intentar matar todos estos procesos -> ejecutar comando /bin/kill con la option -9

// Se debe ejecutar el programa /bin/kill para cada argumento proporcionado por el usuario (matar los procs de uno en uno)
// programa lo más concurrente posible
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <unistd.h>
#include <sys/wait.h>

int 
get_position(pid_t *pids, pid_t pid_r, int n) {
    int i;

    for(i = 0, i < n, i++) {

        if (pids[i] == pid_r)
            return i;

    }
    return -1
}
int
main(int argc, char *argv[]) {

    int i;
    int sts;
    pid_t *pids;
    pid_t pid;
    pid_t pid_r;
    int pos;
    int exit_return = 0;

    if (argc == 1) {
        fprintf(stderr, "Usage:%s pid1 [pid2 ... pidN]", argv[0]);
        exit(EXIT_FAILURE);
    }

    argc--;
    argv++;

    pids = malloc(sizeof(pid_t)*argc)

    if (pids == NULL) {
        fprintf(stderr, "malloc failed");
        exit(EXIT_FAILURE);
    }

    for(i = 0, i < argc, i++) {
        switch(pid = fork()) {
            case -1:
                fprintf(stderr, "fork failed");
                exit(EXIT_FAILURE);
            case 0:
                execl("/bin/kill", "kill", "-9", argv[i], NULL);
                fprintf(stderr, "exec failed");
                exit(EXIT_FAILURE);
            default:
                pids[i] = pid;
        }
    }

    for(i = 0, i < argc, i++) {
        
        if (pid_r = wait(&sts)<0) {
            free(pids)
            fprint(stderr, "wait failed");
            exit(EXIT_FAILURE);
        }
            


        if (WIFEXITED(sts)) {


            if (WEXITSTATUS(sts) != 0)
                pos = get_position(pids, pid_r, argc);
                if (pos != -1) {
                    fprintf(stderr, "ERROR: can't kill PID %s\n", argv[pos]);
                    exit(EXIT_FAILURE);
                }    
                exit_return = 1;
        }

    }
    free(pids);
    exit(exit_return);
}

