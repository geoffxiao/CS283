#include "csapp.h"

/*
	What is the output of the following program?

	fd2 = 4
*/

int main()
{

	int fd1, fd2;
	
	// descriptors 0, 1, 2 are taken by stdin, stdout, and stderr respectively
	
	fd1 = Open("foo.txt", O_RDONLY, 0); // Open "foo.txt", fd1 is descriptor 3
	
	fd2 = Open("bar.txt", O_RDONLY, 0); // Open "bar.txt", fd2 is descriptor 4
	
	Close(fd2); // Close fd2 file, the resources associated with this file are destroyed
	// file descriptor 4 is now available
	// file descriptors 0, 1, 2, 3 are still taken

	fd2 = Open("baz.txt", O_RDONLY, 0); // Open "baz.txt"
	// fd2 is descriptor 4 because 4 is the smallest descriptor available
	
	printf("fd2 = %d\n", fd2); // "fd2 = 4" because fd2 is file descriptor 4
	
	exit(0);

}
