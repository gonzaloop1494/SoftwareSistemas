// Recibe un par de argumentos que describen ficheros y cadenas de texto a buscar dentro de esos ficheros.
// El programa escribe por salida todas las líneas que contienen la cadena dada en el fichero indicado.


// Cada argumento que describa el path de un fichero debe de ir seguido de 
// la cadena de texto que se desea buscar dentro de ese fichero

// si no se proporcionan argumentos o se proporciona un número impar de argumentos, se debe considerar un error

#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// Para buscar las cadenas de texto dentro de los ficheros -> fgrep

// Se debe utilizar fork(2) execl(3) para realizar este programa
// Programa lo más concurrente posible -> creando un proceso que ejecute el comando fgrep 
// por cada búsqueda de una cadena de texto en un fichero (esto es, por cada par path-cadena)

int
main (int argc, char *argv[]) {
    argc -= 1;
    argv += 1;

    int i;
    int sts;

    //Si no se proporcionan argumentos o se proporciona un número impar de argumentos, se debe considerar un error
    if ((argc == 0) || (argc % 2 != 0)) {
        printf("error: bad number of arguments\n");
        exit(EXIT_FAILURE);
    }
    // Una vez comprobada la correcta inserción de argumentos en la línea de comandos
    // Dejamos de tener en cuenta el argv[0] -> nombre de programa

    

    //Recorremos los argumentos que se introducen
    // Vamos saltando argumentos de dos en dos, ya que son path-cadena
    for (i = 0; i < argc; i+=2) {
       switch(fork()){ 
            case -1:
                printf("fork failed\n");
                exit(EXIT_FAILURE);
            case 0:
                execl("/bin/fgrep", "fgrep", argv[i+1], argv[i], NULL);
                printf("exec failed\n");
                exit(EXIT_FAILURE);
       } 
    }
    // Espero los procesos de los hijos
    for (i = 0; i < argc; i += 2) {
        if (wait(&sts) < 0) {
            printf("wait failed\n");
            exit(EXIT_FAILURE);
        }
    }
    exit(EXIT_SUCCESS);
}