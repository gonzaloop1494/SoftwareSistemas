/* Programa que recibe los siguientes argumentos: 
    -> N: Número en base 10 que indica cuántas palabras se deben imprimir. 
    -> L: Número en base 10 que indica a cuántos caracteres se va a truncar cada palabra. 
    -> Palabra: Número indeterminado de ellas (mayor o igual a cero).*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int
str_to_int (char *str, int decimal_base)
{
  char *end = NULL;
  int num;

  num = strtol (str, &end, decimal_base);
  if (*end != '\0' || end == str)
    {
      fprintf (stderr, "error: bad value \"%s\"\n", str);
      exit (EXIT_FAILURE);
    }
  return num;
}

void
n_ok (int *n, int argc)
{
  int n_words;

  n_words = argc - 3;

  if (*n < 0)
    {
      fprintf (stderr, "error: bad value \"%d\"\n", *n);
      exit (EXIT_FAILURE);
    }
  if (*n > n_words)
    {
      *n = n_words;
    }

}

void
l_ok (int l)
{
  if (l < 0)
    {
      fprintf (stderr, "error: bad value \"%d\"\n", l);
      exit (EXIT_FAILURE);
    }
}

void
check_l (int l, char *str)
{
  int len = strlen (str);


  if (l >= len)
    {
      printf ("%s\n", str);
    }
  else
    {
      str[l] = '\0';
      printf ("%s\n", str);
    }


}

// El programa debe imprimir las N últimas palabras, una por línea, en orden de izq a der
// Si N es mayor que el número de argumentos disponibles, se imprimirán todos
int
main (int argc, char *argv[])
{
  int decimal_base = 10;	//Base a la que vamos a convertir en enteros los números de N y L de los argumentos de 
  //la línea de comandos.
  int n;
  int l;
  //int n_words = argc - 3;
  int start;


  if (argc < 3)
    {				// error, si el usuario solo escribe el argv[0] (nombre_programa), y el argv[1]
      fprintf (stderr, "usage: necho N L [word ...]");
      exit (EXIT_FAILURE);
    }

  n = str_to_int (argv[1], decimal_base);
  l = str_to_int (argv[2], decimal_base);


  n_ok (&n, argc);
  l_ok (l);


  start = argc - n;
  for (int i = start; i < argc; i++)
    {
      check_l (l, argv[i]);
    }
  exit (EXIT_SUCCESS);

}
