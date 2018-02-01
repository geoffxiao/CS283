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
#include <string.h>

#define	MAXLINE	 8192  /* max text line length */
#define  MAXFILENAME 128 /* max file name */

int main(int argc, char **argv)
{
	
	char buf[MAXLINE]; // where to store read output, and where to write from
	char output_file[MAXFILENAME][MAXFILENAME]; // output to file named in command-line arguments

	int fd[MAXLINE]; // Output file descriptors
	int append_flag; // Append to output file?

	int file_counter = 0; // number of file arguments counter

	// parse command-line arguments
	for(int i = 1; i < argc; i++)
	{
		// -a file
		if(strcmp(argv[i],"-a") == 0)
		{
			append_flag = 1; // append -a option
		}
		else
		{
			strcpy(output_file[file_counter],argv[i]); // get filename
			file_counter = file_counter + 1;
		}
	}

	// Option flags for open
	// 	O_RDRW = Read + write
	//		O_TRUNC = If file already exists, empty the file
	// 	O_CREAT = If file doesn't exist, create empty version of file
	// 	O_APPEND = append to file
	// 	S_IRUSR = give user read permissions for created file
	// 	S_IWUSR = give user write permissions for created file
	if(append_flag == 1) // append
	{
		for(int i = 0; i < file_counter; i++)
		{
			fd[i] = open(output_file[i], O_RDWR | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
			if(fd[i] < 0)
			{
				fprintf(stderr, "Error opening file %s\n", output_file[i]);
			}
		}
	}
	else // don't append
	{	
		for(int i = 0; i < file_counter; i++)
		{
			fd[i] = open(output_file[i], O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
			if(fd[i] < 0)
			{
				fprintf(stderr, "Error opening file %s\n", output_file[i]);
			}
		}
	}

	int n; // # bytes read = # bytes to write
	// Write to stdout and files
	while((n = read(STDIN_FILENO, buf, MAXLINE)) != 0)
	{
		write(STDOUT_FILENO, buf, n); // write to stdout
		for(int i = 0; i < file_counter; i++)
		{
			write(fd[i], buf, n); // write to file
		}
	}
		
	// Close files
	for(int i = 0; i < file_counter; i++)
	{
		close(fd[i]);
	}

	exit(0);
}
