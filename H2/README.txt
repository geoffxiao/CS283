Geoffrey Xiao
CS283 
H2
--------------------------------------------------------------------------------------------------
All code compiled using tux (gcc 5.4.0 Ubuntu 5.4.0-6ubuntu~16.04.4)
--------------------------------------------------------------------------------------------------
Descriptions/Requirements to run/Instructions:

Problem 1. 
Run program to test output.

Assumptions: Nothing to note.


Problem 2.
Takes commandline argument, a file descriptor number. Outputs the type and
read access of the file. 

Assumptions: None
Instructions:
Run program as...
	program arg
		program: executable program
		arg = a file descriptor number


Problem 3.
Copies stdin to stdout. If commandline argument specified, copy from the
commandline agument file to stdout.

Assumptions: None
Instructions:
Run program as...
	program arg
		program: executable program
		arg(optional) = file to copy to stdout, if not set, copies from stdin


Problem 4. 
Assumptions: Max bytes in one line of input from stdin is 8192. Max filename
argument is 128 bytes. Max number of commandline file arguments is 128.
Instructions:
Run the program as...
	program [options] arg
		program: exectuable program
		[options]: -a = append to file, default is to truncate file
		arg: file to write to, can pass multiple file args
--------------------------------------------------------------------------------------------------
Testing:

Problem 1. Ran program to confirm output.

Problem 2. Tested on stdin, stdout, and stderr. stdin, stdout, and stderr
were file type "other" and read "yes".

Problem 3. Tested with stdin. Also tested with commandline arguments for
input files.

Problem 4. Compared program behavior with bash tee command.
Tested by comparing with results of Unix tee command.

All programs, when tested, ran correctly.
