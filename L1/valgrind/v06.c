#include <stdio.h>

int foo (x)
int x;
 {
  if(x < 10)
   {
    printf("x is less than 10\n");
   }
 }

int main()
 {
  int y = 0; /* changed from 'int y' to 'int y = 0'. Need to initialize. */
  foo(y);
 }
