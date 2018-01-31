/*
	The tee command reads its standard input until end-of-file, writing a copy of the
	input to standard output and to the file named in its command-line argument.
	Implement tee using I/O system calls. By default, tee overwrites any existing file with
	the given name. Implement the -a command-line option (tee -a file), which causes
	tee to append text to the end of a file if it already exists. (Use the getopt() function
	or write your own to parse command-line options.) 
*/

#include <fcntl.h> // For Unix system calls
#include <unistd.h> // For Unix system calls
#include <stdio.h>
#include <stdlib.h>

#define	MAXLINE	 8192  /* max text line length */

int main(int argc, char **argv)
{
	
	char buf[MAXLINE]; // where to store read output, and where to write from
	char* output_file = argv[1]; // output to file named in command-line arguments
	
	int fd; // Output file descriptor
	int append_flag = 0; // Append to output file?

	// Parse command-line arguments using getopt
	int c;
	while ((c = getopt (argc, argv, "a:")) != -1)
	{
		switch (c)
		{
			// -a option
			case 'a':
				append_flag = 1;
				output_file = optarg; // argument associated with -a option
				break;
		
			// Error checking, invalid arg
			case '?':
				exit(1);
   	 }
	}

	// Do we have a file argument?
	if(optind >= argc && append_flag == 0)
	{
		fprintf(stderr, "No file argument passed\n");
		exit(1);
	}
	else if(append_flag == 0) 
	// get the file arg, file arg not associated with option flag
	{
		output_file = argv[optind];
	}
	

	// Option flags for open
	// 	O_RDRW = Read + write
	//		O_TRUNC = If file already exists, empty the file
	// 	O_CREAT = If file doesn't exist, create empty version of file
	// 	O_APPEND = append to file
	// 	S_IRUSR = give user read permissions for created file
	// 	S_IWUSR = give user write permissions for created file
	if(append_flag == 1) // Append
		fd = open(output_file, O_RDWR | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
	else // Don't append
		fd = open(output_file, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
	
	if(fd < 0)
		fprintf(stderr, "Error opening file %s for writing", output_file);

	int n; // # bytes read = # bytes to write
	// Write to stdout and file
	while((n = read(STDIN_FILENO, buf, MAXLINE)) != 0)
	{
		write(STDOUT_FILENO, buf, n); // write to stdout
		write(fd, buf, n); // write to file
	}
		
	// Close file
	close(fd);

	exit(0);
}
