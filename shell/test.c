#include <sys/types.h>
#include <sys/wait.h>       
#include <errno.h>
#include <stdio.h>
#include <string.h>

int main()
{
	int status;
	int i = waitpid(-1, &status, 0);

	printf("%s\n", strerror(errno));
	return 0;
}
