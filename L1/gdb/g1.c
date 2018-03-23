#include <ctype.h> 
#include <stdio.h> 

int main () 
 {
  char c;
  c = fgetc (stdin) ;
  while (c != EOF) 
  { /* added bracket */
  if (isalnum (c) ) 
   printf ("%c\n", c) ;
  c = fgetc (stdin) ;
  } /* added bracket */
  return(1);
 }
