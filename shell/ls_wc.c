      
#include <stdio.h>       
#include <unistd.h>
#include <sys/types.h>


int main(void)
 {
 int pfds[2];
 pipe(pfds);
 if (!fork())
 {
 close(pfds[0]); /* we don't need this */
 dup2(pfds[1], 1);
 execlp("ls", "ls", NULL);
 }
 else
 {
 close(pfds[1]); /* we don't need this */
 dup2(pfds[0], 0);
 execlp("wc", "wc", "-l", NULL);
 }
 return 0;
 }
