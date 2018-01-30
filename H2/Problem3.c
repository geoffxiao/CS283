#include "csapp.h"

int main(int argc, char **argv)
{
	
	int n;
	rio_t rio;
	char buf[MAXLINE];

	// ---- Inserted code below ---- //
	int fd = -1;

	if(argc > 1) // Optional command line argument
	{
		fd = Open(argv[1], O_RDONLY, 0); // Open infile
		dup2(fd, STDIN_FILENO); // Redirect file to stdin
		printf("%d", fd);
	}
	// ---- Inserted code above ---- //

	Rio_readinitb(&rio, STDIN_FILENO);
	while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
		Rio_writen(STDOUT_FILENO, buf, n);

	// ---- Inserted code below ---- //
	// Close file if it was opened
	if(fd != -1)
		Close(fd);

	exit(0);
}
