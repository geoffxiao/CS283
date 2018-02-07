Geoffrey Xiao
CS283 
H3

--------------------------------------------------------------------------------------------------

All code compiled using tux (gcc 5.4.0 Ubuntu 5.4.0-6ubuntu~16.04.4)

--------------------------------------------------------------------------------------------------
Descriptions/Requirements to run/Instructions:

Takes 3 commandline arguments, `find`, `replace`, and `prefix`. The Makefile compiles 'H3.c' into 
the executable 'H3'.
	To run: >> H3 find replace prefix

Assumptions: Maximum size of one line in file = 2048 bytes. The program also creates a temporary 
             file called 'H3-temp-file' in the current directory.

For all '.txt' files in the current directory,
	- The program replaces all occurrences of `find` with `replace`
	- If there are no occurences of `find`, the program prepends `find` to the left of any
	  ocurrence of `prefix`
	- If there are no occurrences of `find` or `prefix`, the file is unchanged
	- The program modifies the '.txt' files in the current directory


--------------------------------------------------------------------------------------------------
Program Structure:
1. Loop through all .txt files in current directory
2. Scan each .txt file line by line.
3. For each line, use the function replace_all. This replaces all instaces of `find` with `replace.
4. Write processed to line to temporary file. 
5. After processing all lines, replace original file with the temporary file.
6. If no replacements were made, reopen the .txt file.
   6.a. For each line, use the function prepend_all. This prepends `find` to the left of all 
        instanes of `prefix`.
   6.b. Write processed line to temporary file.
   6.c. After processing all lines, replace original file with the temporary file.
7. Program finished.


--------------------------------------------------------------------------------------------------
Testing:

To test the program I wrote a Bash script that performs the same procedures as the C program. The 
script uses the Bash `sed` command. The Bash script replaces `find` with `replace` and if 
there are no instances of `find`, the script replaces `prefix` with `find``prefix`. The Bash 
script then puts the processed files into a folder called 'testing'. 

The Bash script is called 'bash-H3' and takes the same three commandline arguments as my program 'H3'.
	To run: >> bash-H3 find replace prefix


The Bash script 'Test' takes three commandline arguments `find`, `replace`, and `prefix`. 'Test'
runs my 'H3' C program and the 'bash-H3' Bash script using these arguments and compares the results
using `diff`. If there is no output from 'Test' the C program and Bash script produced the same files.
The stdout log from the 'H3' C program are placed into a file called H3-output.
	To run: >> Test find replace prefix


When I used 'Test' on various files, the Bash and C programs gave the same results.