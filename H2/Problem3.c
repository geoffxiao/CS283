/*
	Modify the cpfile program in Figure 10.5 so that it takes an optional 
	command line argument infile. If infile is given, then copy infile to 
	standard output; otherwise, copy standard input to the standard output 
	as before. The twist is that your solution must use the original copy 
	loop for both cases. You are only allowed to insert code, and you are 
	not allowed to change any of the existing code.
*/
#include "csapp.h"

int main(int argc, char **argv)
{
	
	int n;
	rio_t rio;
	char buf[MAXLINE];

	// ---- Inserted code below ---- //
	if(argc > 2) // Check number of args
	{
		fprintf(stderr, "Invalid number of arguments\n");
		exit(1);
	}

	if(argc == 2) // Optional command line argument
	{
		int fd = Open(argv[1], O_RDONLY, 0); // Open infile
		if(fd < 0)
		{
			fprintf(stderr, "Unable to read %s\n", argv[1]);
			exit(1);
		}
		Dup2(fd, STDIN_FILENO); // Redirect file to stdin using Dup2 from csapp
		Close(fd);
	}
	// ---- Inserted code above ---- //

	Rio_readinitb(&rio, STDIN_FILENO);
	while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
		Rio_writen(STDOUT_FILENO, buf, n);

	exit(0);
}
