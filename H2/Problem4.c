/*
	The tee command reads its standard input until end-of-file, writing a copy of the
	input to standard output and to the file named in its command-line argument.
	Implement tee using I/O system calls. By default, tee overwrites any existing file with
	the given name. Implement the -a command-line option (tee -a file), which causes
	tee to append text to the end of a file if it already exists. (Use the getopt() function
	or write your own to parse command-line options.) 
*/

include "csapp.h"

int main(int argc, char **argv)
{
	
	int n;
	rio_t rio; // Robust I/O
	char buf[MAXLINE]; // where to store read output, and where to write from
	char* output_file = argv[1]; // output to file named in command-line arguments
	
	int fd;
	int append_flag = 0;
	
	// Parse command-line arguments using getopt
	int c;
	while ((c = getopt (argc, argv, "a:")) != -1)
    switch (c)
	{
		// -a option
		case 'a':
			append_flag = 1;
			output_file = optarg;
			break;
		
		case '?':
			fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
			exit(1);

		default:
			exit(1);
    }

	// Parse non option arguments
	for (index = optind; index < argc; index++)
	{
		if(append_flag == 0)
		{
			output_file = argv[index];
		}
	}		
	
	// O_RDRW = Read + write
	// O_TRUNC = If file already exists, empty the file
	// O_CREAT = If file doesn't exist, create empty version of file
	// O_APPEND = append to file
	if(append_flag == 0)
	{
		fd = Open(output_file, O_RDWR | O_APPEND | O_CREAT)
	}		
	else // Don't append
	{
		fd = Open(output_file, O_RDWR | O_TRUNC | O_CREAT)
	}		
	
	
	// Error check - Try to open the file
	if(fd) < 0)
	{	
		printf("Invalid/Missing Commandline Argument");
	}
	
	// Write to stdout and file
	Rio_readinitb(&rio, STDIN_FILENO);
	while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
	{
		Rio_writen(STDOUT_FILENO, buf, n); // write to stdout
		Rio_writen(fd, buf, n); // write to file
	}
		
	// Close file if it was opened
	if(fd != -1)
		Close(fd);

	exit(0);
}