#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

enum{Bufsize = 8*1024}; 

int
read_file(int fd, char *buffer) {
    int nr;
    int total = 0;

    while ((nr = read(fd, buffer + total, Bufsize - total)) > 0) {
        total += nr;
    }

    if (nr < 0) {
        close (fd);
        err(EXIT_FAILURE, "read failed\n");
    }

    return total;
}

int
write_file(int fd, char *buffer, int len) {
    int total = 0;
    int wb = 0;

    while (total < len) {
        wb = write(fd, buffer + total, len - total);

        if (wb < 0) {
            close(fd);
            err(EXIT_FAILURE, "writing failed\n");
        }

        if (wb == 0) {
            break;
        }

        total += wb;
    }

    return total;
}

int
main (int argc, char *argv[]) {

    int fdin;
    int fdout;
    long nbytes;
    char *end;
    int base_decimal = 10;
    int nfich = 0; //número/índice del fichero por el que voy leyendo
    int index = 0;
    char *filenameout;
    int nr;
    char buffer[Bufsize];
    char *bufferaux;

    if (argc != 3) {
        errx(EXIT_FAILURE, "Usage: %s N file", argv[0]);
    }

    nbytes = strtol(argv[1], &end, base_decimal);
    if (end == argv[1] || (*end != '\0')) {
        errx(EXIT_FAILURE, "no digits have been found\n");
    }
    if (nbytes < 0) {
        errx (EXIT_FAILURE, "error reading bytes given\n");
    }

    filenameout = (char *) malloc((strlen(argv[2]) + 3) * sizeof(char) + 1);
    if (!filenameout) {
        errx (EXIT_FAILURE, "malloc failed\n");
    }

    bufferaux = (char *) malloc(nbytes * sizeof(char));
    if (!bufferaux) {
        errx (EXIT_FAILURE, "malloc failed\n");
    }

    fdin = open(argv[2], O_RDONLY);
    if (fdin < 0) {
        err(EXIT_FAILURE, "error open\n");
    }

    nr = read_file(fdin, buffer);
    close(fdin);

    while (nr >= nbytes) {

        snprintf(filenameout, 256, "%03d%s", nfich, argv[2]);
        nfich++;
        fdout = open(filenameout, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fdout < 0) {
            err(EXIT_FAILURE, "%s: opening failed\n", filenameout);
        }

        memcpy(bufferaux, buffer + index, nbytes);

        write_file(fdout, bufferaux, nbytes);

        close(fdout);
        bufferaux[0] = '\0';
        filenameout[0] = '\0';
        index += nbytes;
        nr -= nbytes;
    }

    if (nr != 0) {
        snprintf(filenameout, 256, "%03d%s", nfich, argv[2]);
        nfich++;
        fdout = open(filenameout, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fdout < 0) {
            err(EXIT_FAILURE, "%s: opening failed\n", filenameout);
        }

        memcpy(bufferaux, buffer + index, nr);

        write_file(fdout, bufferaux, nr);

        close(fdout);
        bufferaux[0] = '\0';
        filenameout[0] = '\0';
        index += nr;
        nr = 0;
    }

    free(bufferaux);
    free(filenameout);

    exit(EXIT_SUCCESS);
}
