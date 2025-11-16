#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

void
myls(char *dirname) {
    DIR *d;
    struct dirent *entry;
    struct stat st;
    char t; // tipo de archivo

    d = opendir(dirname);
    if (d == NULL) {
        err(EXIT_FAILURE, "opendir failed");
    }

    while((entry = readdir(d)) != NULL ) {

        // lstat para NO seguir enlaces simbólicos
        if (lstat(entry->d_name, &st) < 0) {
            warn("lstat failed on %s", entry->d_name);
            continue;
        }

        // Determinar el tipo de archivo
        if (S_ISDIR(st.st_mode)) {
            t = 'd'; // directorio
        }
        else if (S_ISREG(st.st_mode)) {
            t = 'f'; // fichero regular
        }
        else if (S_ISLNK(st.st_mode)) {
            t = 'l'; // enlace simbólico
        }
        else
            t = 'o'; // otro tipo

        // Imprimir toda la información pedida: tipo, uid, gid, tamaño, nombre
        printf("%c\t%u\t%u\t%lld\t%s\n",
               t,
               st.st_uid,
               st.st_gid,
               (long long) st.st_size,
               entry->d_name);
    }

    closedir(d);
}

int 
main(int argc, char *argv[]) {
    
    if (argc != 1) {
        errx(EXIT_FAILURE, "Usage: %s", argv[0]);
    }

    myls(".");

    exit(EXIT_SUCCESS);
}
