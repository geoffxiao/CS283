#include <stdio.h> 
#include <stdlib.h> 

int main () 
 {
  char * buf;
  int num;
  num = 1024; /* change from 1 << 31 to 1024 to avoid null pointer */
  buf = malloc(num) ;
  fgets (buf, 1024, stdin) ;
  printf ("%s\n", buf) ;
  return(1);
 }
