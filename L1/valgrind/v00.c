#include <stdlib.h>

void f ()
 {
  int *x;
  x = malloc(10 * sizeof(int));
  x[9] = 0; /* changed from x[10] */
  free(x); /* need to free the memory */
 }

int main ()
 {
  f();
  return 0;
 }
