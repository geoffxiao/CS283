#include <sys/types.h>
#include <sys/wait.h>       
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include<unistd.h>


void handler(int sig)
{
	printf("Handler\n");
}

int main()
{
	signal(SIGCHLD, handler);

	srand(time(NULL));
	for(int j = 0; j < 5; j++)
	{
		// Create child and have child sleep
		if(fork() == 0)
		{
			printf("Child %d\n", j);
			exit(0);
		}
		printf("%d_%d\n", j, getpid());
	}
	
	printf("Parent\n");

	int status;
	while( waitpid(-1, &status, 0) > 0 )
	{
		printf("Reaping\n");
	}
	return 0;
}
