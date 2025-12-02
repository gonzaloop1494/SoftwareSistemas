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

    if (argc != 2)
        errx(EXIT_FAILURE, "Usage: %s pid", argv[0]);

    if (pipe(p) == -1)
        err(EXIT_FAILURE, "pipe failed");

    /* ----- Primer hijo: ejecuta ps aux ----- */
    switch (fork()) {
        case -1:
            err(EXIT_FAILURE, "fork failed");

        case 0:
            dup2(p[1], 1);      // stdout → pipe
            close(p[0]);        // cerrar lectura
            close(p[1]);        // cerrar escritura después de dup2
            execl("/usr/bin/ps", "ps", "aux", NULL);
            err(EXIT_FAILURE, "exec failed");
    }

    /* ----- Segundo hijo: analiza la salida de ps ----- */
    switch (fork()) {
        case -1:
            err(EXIT_FAILURE, "fork failed");

        case 0:
            close(p[1]);        // no escribe
            fp = fdopen(p[0], "r");
            if (fp == NULL)
                err(EXIT_FAILURE, "fdopen failed");

            while (fgets(buf, BUF, fp) != NULL) {
                tokenize(buf, tokens, " \t", MAXTOK);

                // Asegúrate de que la segunda columna contiene el PID
                if (tokens[1] && strcmp(tokens[1], argv[1]) == 0) {
                    fclose(fp);
                    exit(0);    // encontrado → éxito
                }
            }

            fclose(fp);
            exit(1);            // no encontrado → fallo
    }

    /* ----- Padre ----- */
    close(p[0]);
    close(p[1]);

    int status_ps, status_checker;

    wait(&status_ps);     // esperar hijo ps
    wait(&status_checker);// esperar hijo lector

    if (WIFEXITED(status_checker))
        exit(WEXITSTATUS(status_checker));

    exit(EXIT_FAILURE);   // por seguridad
}
