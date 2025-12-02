#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <sys/wait.h>
#include <string.h>

enum { BUF = 512, MAXTOK = 32 };

int tokenize(char *str, char *tokens[], char *delim, int max)
{
    int i = 0;
    char *p;

    while ((p = strtok_r(str, delim, &str)) != NULL && i < max) {
        tokens[i++] = p;
    }
    return i;
}

int main(int argc, char *argv[])
{
    int p[2];
    char buf[BUF];
    FILE *fp;
    char *tokens[MAXTOK];
    int exit_status = 1;  // Por defecto, si no se encuentra el PID, termina con 1
    int sts;
    pid_t pidps, pidchecker;

    if (argc != 2)
        errx(EXIT_FAILURE, "Usage: %s pid", argv[0]);

    if (pipe(p) == -1)
        err(EXIT_FAILURE, "pipe failed");

    // Primer hijo: ejecuta el comando ps aux
    switch (pidps = fork()) {
        case -1:
            err(EXIT_FAILURE, "fork failed");

        case 0:
            dup2(p[1], 1);      // Redirige stdout al pipe
            close(p[0]);        // Cierra la lectura del pipe en el hijo
            close(p[1]);        // Cierra la escritura después de redirigir
            execl("/usr/bin/ps", "ps", "aux", NULL);
            err(EXIT_FAILURE, "exec failed");
    }

    // Segundo hijo: analiza la salida de ps
    switch (pidchecker = fork()) {
        case -1:
            err(EXIT_FAILURE, "fork failed");

        case 0:
            close(p[1]);        // No necesitamos escribir en el pipe
            fp = fdopen(p[0], "r");
            if (fp == NULL)
                err(EXIT_FAILURE, "fdopen failed");

            while (fgets(buf, BUF, fp) != NULL) {
                tokenize(buf, tokens, " \t", MAXTOK);  // Tokenizar por espacio o tabulador

                // Verificamos si la segunda columna (PID) es igual al PID pasado como argumento
                if (tokens[1] && strcmp(tokens[1], argv[1]) == 0) {
                    exit_status = 0;  // PID encontrado, salimos con éxito
                    break;
                }
            }

            fclose(fp);
            exit(exit_status);
    }

    // Padre: cierra los descriptores de archivo que ya no necesita
    close(p[0]);
    close(p[1]);

    // Espera a los hijos
    int status_ps, status_checker;
    waitpid(pidps, &status_ps, 0);       // Espera al hijo que ejecuta ps
    waitpid(pidchecker, &status_checker, 0);  // Espera al hijo que analiza la salida

    if (WIFEXITED(status_checker)) {
        exit(WEXITSTATUS(status_checker));  // Propaga el estado de salida del hijo que verifica el PID
    }

    exit(EXIT_FAILURE);   // Si todo falla, se sale con 1
}
