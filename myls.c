#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <dirent.h>
#include <sys/stat.h>


void
myls(char *dirname) {
    DIR *d;
    struct dirent *entry;
    struct stat st;
    char t; // Esta variable almacena el tipo de archivo

    d = opendir(dirname);
    if (d == NULL) {
        err(EXIT_FAILURE, "opendir failed");
    }

    while((entry = readdir(d)) != NULL ) {
        //printf("%s\n", entry ->d_name);

        //comprobamos el tipo de archivo
        if (stat(entry->d_name,&st)){ 
			err(EXIT_FAILURE,"error stat");
        }
        

        // Determinar el tipo de archivo
		if (S_ISDIR(st.st_mode)) {
			t = 'd'; //directorio
        }
		else if (S_ISREG(st.st_mode)) {
			t = 'f'; //archivo regular
        }
		else if (S_ISLNK(st.st_mode)) {
			t = 'l'; //enlace simbólico
        }
		else
			t = 'o'; // Otro tipo de archivo

        // Imprimir información del archivo
        printf("%c\t%lld\t%s\n", t, st.st_size, entry->d_name);
		
		
		
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
