#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

enum { Bufsize = 512, MAXTOKENS = 32 };

int tokenize(char *str, char *tokens[], char *delim, int max) {
    int i = 0;
    char *p;
    
    while (((p = strtok_r(str, delim, &str)) != NULL) && (i < max)) {
        tokens[i] = p;
        i++;
    }
    return i;
}

// Función para leer la salida de ps y verificar si el PID está presente
void check_pid_in_ps_output(int pipe_fd, const char *pid_to_check) {
    FILE *fp;
    char buf[Bufsize];
    char *tokens[MAXTOKENS];
    int exit_status = 1;

    // Abrir el pipe como archivo
    fp = fdopen(pipe_fd, "r");
    if (fp == NULL) {
        err(EXIT_FAILURE, "fdopen failed");
    }

    // Leer línea por línea de la salida de ps
    while ((fgets(buf, Bufsize, fp) != NULL) && (exit_status)) {
        tokenize(buf, tokens, " ", MAXTOKENS);
        if (strcmp(tokens[1], pid_to_check) == 0) {
            exit_status = 0; // PID encontrado
        }
    }
    fclose(fp);
    exit(exit_status); // Salir con éxito o error
}

int main(int argc, char *argv[]) {
    int p[2]; // Pipe para comunicación entre procesos
    int exit_status;

    // Control de errores
    if (argc != 2) {
        errx(EXIT_FAILURE, "Usage: %s pid", argv[0]);
    }

    // Crear el pipe
    if (pipe(p) == -1) {
        err(EXIT_FAILURE, "pipe failed");
    }

    // Primer fork para ejecutar el comando ps
    switch (fork()) {
        case -1:
            err(EXIT_FAILURE, "fork failed");
        
        case 0: // Hijo que ejecuta el comando ps con los parámetros aux
            dup2(p[1], 1); // Redirigir la salida estándar al pipe
            close(p[0]); // Cerrar el extremo de lectura del pipe
            close(p[1]); // Ya hemos redirigido la salida
            execl("/usr/bin/ps", "ps", "aux", NULL); // Ejecutar ps aux
            err(EXIT_FAILURE, "exec failed"); // Si execl falla
    }

    // Segundo fork para leer la salida de ps
    switch (fork()) {
        case -1:
            err(EXIT_FAILURE, "fork failed");

        case 0: 
            close(p[1]); // Cerrar el extremo de escritura del pipe
            check_pid_in_ps_output(p[0], argv[1]); // Llamar a la función para verificar el PID
    }

    // En el proceso padre, cerramos ambos extremos del pipe y esperamos a los hijos
    close(p[0]);
    close(p[1]);
    wait(NULL); // Esperar al primer hijo (ps)
    wait(NULL); // Esperar al segundo hijo (lector)

    exit(EXIT_SUCCESS);
}
