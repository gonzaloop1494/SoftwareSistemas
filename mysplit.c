#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <fcntl.h>
#include <unistd.h>
int
main (int argc, char *argv[]) {

    int fdin;
    int fdout;
    long nbytes;
    char *end;
    int base_decimal = 10;
    int nfich = 0; //número/índice del fichero por el que voy leyendo
    int index = 0;
    int byteswritten = 0;
    char filenameout[256];
    int nr;
    char buffer[Bufsize];

    if (argc != 3) {
        errx(EXIT_FAILURE, "Usage: %s N file", argv[0]);
    }

    nbytes = strtol(argv[1], &end, base_decimal)

    if (end == argv[1] || (*end != '\0')) {
        errx(stderr, "no digits have been found\n");
    }
    if (nbytes < 0) {
        errx (EXIT_FAILURE, "error reading bytes given\n");
    }

    fdin = open(argv[2], O_RDONLY);

    if (fdin < 0) {
        err(EXIT_FAILURE, "error open\n");
    }

    byteswritten = nbytes;
    while ((nr = read(fd, buffer, Bufsize) != 0)) {
        if (nr < 0) {
            err(EXIT_FAILURE, "read failed\n");
        }

        index = 0;
        while (nr > 0){

            if (byteswritten == nbytes){// primera iteración
                snprintf(filenameout, 256, "%03d%s", nfich, argv[2]);
                nfich++;
                fdout= open(filenameout, O_WRONLY | O_CREAT | O_TRUNC, 0664);
                if (fdout < 0) {
                    err(EXIT_FAILURE, "%s: opening failed\n", filenameout);
                }
            if (byteswritten <= nr) {
                 if (write(fdout, &buffer[index], byteswritten) != byteswritten) {
                    err(EXIT_FAILURE, "writing failed\n");
                    close(fdout);
                    index += byteswritten; 
                    nr -= byteswritten;
                    byteswritten = nbytes;
                 } 

            }
            else
            {
                if (write(fdout, &buffer[index], nr) != nr) {
                    err(EXIT_FAILURE, "writing failed\n");
                }
                byteswritten -= nr;
                nr = 0;
            }

            } 

        }
    }

    close(fdin);
    close(fdout);
    exit(EXIT_SUCCESS);
}