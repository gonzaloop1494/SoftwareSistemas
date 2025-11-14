#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <dirent.h>



void
myls(char *dirname) {
    DIR *d;
    struct dirent *entry;
    struct stat st;

    d = opendir(dirname);
    if (d == NULL) {
        err(EXIT_FAILURE, "opendir failed");
    }

    while((entry=readdir(d)) != NULL ) {
        printf("%s\n", entry -> d_name);
        if (stat(entry->d_ name,&st))
			err(EXIT_FAILURE,"error stat");
		if (S_ISDIR(st.st_mode))
			t='d';
		else if (S_ISREG(st.st_mode))
			t='f';
		else if (IS_ISLNK(st.st_mode))
			t='l';
		else
			t='o';
    
		
		
		
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
