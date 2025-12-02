#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <unistd.h> 
#include <sys/wait.h>
#include <string.h>

enum {Bufsize = 512, MAXTOKENS=32};



int
tokenize(char *str,char *tokens[],char *delim,int max)
{
	int i=0;
	char *p;
	
	while (((p=strtok_r(str,delim,&str)) !=NULL) && (i<max))
	{
		tokens[i]=p;
		i++;
		
	}
	return i;
}




int
main(int argc, char *argv[]) {

    int p[2]; //creo el pipe
    char buf[Bufsize];
    FILE *fp;
    char *tokens[MAXTOKENS];
    int exit_status;

    // Control de errores
    if (argc != 2) {
        errx(EXIT_FAILURE, "Usage: %s pid", argv[0]);
    }
    pipe(p);
    switch(fork()) {

        case -1:
            err(EXIT_FAILURE, "fork failed");
        
        case 0: //hijo que ejecuta el comando ps con los parámetros aux
        dup2(p[1], 1);
        close(p[0]);
        close(p[1]);
        execl("/usr/bin/ps", "ps", "aux", NULL);
        err(EXIT_FAILURE, "exec failed");
    }

    switch(fork()) {
        case -1:
            err(EXIT_FAILURE, "fork failed");

        case 0:
            close(p[1]);
			fp=fdopen(p[0],"r");
			if (fp==NULL)
				err(EXIT_FAILURE,"fdopen failed");
			exit_status=1;
			while((fgets(buf,Bufsize,fp)!=NULL) && (exit_status))
			{
				tokenize(buf,tokens," \t",MAXTOKENS);
				if (strcmp(tokens[1],argv[1])==0)
					exit_status=0;
			    } else {
                    exit_status = -1;
                }
			fclose(fp);
			exit(exit_status);


    }

    


    close(p[0]);
	close(p[1]);
	wait(NULL);
	wait(NULL);
    exit(EXIT_SUCCESS);

}