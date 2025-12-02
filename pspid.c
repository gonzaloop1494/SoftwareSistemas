#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <sys/wait.h>
#include <string.h>

enum { Bufsize = 512, MAXTOKENS = 32 };

// Función original proporcionada
int tokenize(char *str, char *tokens[], char *delim, int max)
{
    int i = 0;
    char *p;

    // Nota: El uso de &str como saveptr funciona porque actualizas 
    // el puntero str para la siguiente iteración en el bucle.
    while (((p = strtok_r(str, delim, &str)) != NULL) && (i < max))
    {
        tokens[i] = p;
        i++;
    }
    return i;
}

/**
 * Función para el primer hijo: Ejecuta el comando ps
 * Recibe el array de la tubería para configurar los descriptores.
 */
void ejecutar_ps(int p[2])
{
    // Redirigir stdout al extremo de escritura del pipe
    if (dup2(p[1], STDOUT_FILENO) == -1)
        err(EXIT_FAILURE, "dup2 failed");

    // Cerrar los descriptores originales del pipe
    close(p[0]);
    close(p[1]);

    execl("/usr/bin/ps", "ps", "aux", NULL);
    
    // Si llegamos aquí, exec falló
    err(EXIT_FAILURE, "exec failed");
}

/**
 * Función para el segundo hijo: Lee la salida y busca el PID
 * Devuelve 0 si lo encuentra, 1 si no.
 */
int buscar_pid(int p[2], char *target_pid)
{
    FILE *fp;
    char buf[Bufsize];
    char *tokens[MAXTOKENS];
    int exit_status = 1;

    // Cerrar el extremo de escritura, no lo necesitamos
    close(p[1]);

    // Abrir el extremo de lectura como stream
    fp = fdopen(p[0], "r");
    if (fp == NULL)
        err(EXIT_FAILURE, "fdopen failed");

    while (fgets(buf, Bufsize, fp) != NULL)
    {
        tokenize(buf, tokens, " ", MAXTOKENS);
        
        // tokens[1] es típicamente el PID en ps aux
        if (tokens[1] != NULL && strcmp(tokens[1], target_pid) == 0)
        {
            exit_status = 0;
            break; // Ya lo encontramos, podemos salir del bucle
        }
    }

    fclose(fp); // Esto también cierra p[0] internamente
    return exit_status;
}

int main(int argc, char *argv[])
{
    int p[2];
    int exit_status = 1;
    int sts;
    pid_t pidson; // Cambiado a pid_t para ser más estricto
    pid_t pidexit;

    if (argc != 2)
        errx(EXIT_FAILURE, "Usage: %s pid", argv[0]);

    if (pipe(p) == -1)
        err(EXIT_FAILURE, "pipe failed");

    // --- Primer Hijo: Productor (ps) ---
    switch (fork())
    {
        case -1:
            err(EXIT_FAILURE, "fork failed");
        case 0:
            ejecutar_ps(p);
            // No necesitamos break o exit aquí porque ejecutar_ps hace exec o sale con error
            break;
    }

    // --- Segundo Hijo: Consumidor (grep lógico) ---
    switch (pidson = fork())
    {
        case -1:
            err(EXIT_FAILURE, "fork failed");
        case 0:
            // La función devuelve 0 o 1, salimos con ese valor
            exit(buscar_pid(p, argv[1]));
    }

    // --- Padre ---
    // Importante cerrar los pipes en el padre para que los hijos reciban EOF
    close(p[0]);
    close(p[1]);

    while ((pidexit = wait(&sts)) != -1)
    {
        if (WIFEXITED(sts))
        {
            // Solo nos interesa el código de salida del segundo hijo (el que busca)
            if (pidexit == pidson)
                exit_status = WEXITSTATUS(sts);
        }
    }

    return exit_status;
}