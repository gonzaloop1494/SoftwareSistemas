#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

enum { Bufsize = 512, MAXTOKENS = 32 };

// Función para tokenizar la línea de la salida de ps
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

    // Variable para controlar si ya hemos procesado la cabecera
    int header_skipped = 0;

    // Leer línea por línea de la salida de ps
    while (fgets(buf, Bufsize, fp) != NULL) {
        // Si es la cabecera (primera línea), la ignoramos
        if (!header_skipped) {
            header_skipped = 1;
            continue;
        }

        // Tokenizar la línea (separar por espacios y tabulaciones)
        int token_count = tokenize(buf, tokens, " \t", MAXTOKENS);

        // Asegurarse de que haya suficientes tokens para comparar (al menos 2: PID y otros campos)
        if (token_count > 1) {
            // Limpiar posibles espacios al principio y final de la cadena (trim)
            char *pid = tokens[1];
            while (isspace((unsigned char)*pid)) pid++; // Eliminar espacios a la izquierda
            char *end = pid + strlen(pid) - 1;
            while (end > pid && isspace((unsigned char)*end)) end--; // Eliminar espacios a la derecha
            *(end + 1) = '\0'; // Terminar la cadena correctamente

            // Comparar el PID (debería estar en la segunda columna)
            if (strcmp(pid, pid_to_check) == 0) {
                exit_status = 0; // PID encontrado
                break;  // Salir del bucle si el PID es encontrado
            }
        }
    }

    fclose(fp);
    exit(exit_status); // Salir con éxito (0) o error (1)
}

int main(int argc, char *argv[]) {
    int p[2]; // Pipe para comunicación entre procesos

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
