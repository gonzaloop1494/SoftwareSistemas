// Programa que reciba un path de dos comandos sin argumento
// el path se ejecuta tal cual (sin mirar $PATH ni nada).

// El comando ejecutará los dos comandos como parámetro concurrentemente y esperará a que acaben

// al final escribe un mensaje con el nombre del comando que haya acabado primero

#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

enum {
    MAX_CMDS = 2
};


int
get_position(int pid_f, int pids[]) {
    int i;

    for(i=0;i<MAX_CMDS;i++){
		if (pids[i]==pid_f)
			return i;
	}
	return -1;
	
}



int
main(int argc, char *argv[]) {

    int i;
    int pid, sts, pid_f, id_pos;
    int pid[MAX_CMDS];
    int status_returned = 0;

    // el programa recibe dos comandos más el nombre del programa
    if (argc != MAX_CMDS + 1) {
        fprintf(stderr, "Usage: %s cmd1 cmd2", argv[0]);
        exit(EXIT_FAILURE);
    }
    // Quitamos el nombre del programa
    argc--;
    argv++;

    for (i = 0; i < argc; i++) {
        switch(pid = fork()) {
            case -1:
                fprintf("fork failed");
                exit(EXIT_FAILURE);
            case 0:
                execl(argv[i], argv[i], NULL);
                fprintf(stderr, "exec failed");
                exit(EXIT_FAILURE);
            default:
                pids[i] = pid;
        }
    }

    for (i = 0; i < argc; i++) {
        if (pid_f = wait(&sts)< 0) {
            fprintf(stderr, "wait failed");
            exit(EXIT_FAILURE);
        }
        if (WIFEXITED(sts)) {
            status_returned = status_returned | WEXITSTATUS(sts);
            if (i == 0) {
                id_pos = get_position(pid_f, pids);
            }
        }
    }

    printf("#### First cmd: %s\n",argv[id_pos]);
    exit(status_returned);
}