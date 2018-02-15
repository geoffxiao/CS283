Geoffrey Xiao
CS2813 
H13

--------------------------------------------------------------------------------------------------

All code compiled using tux (gcc 5.4.0 Ubuntu 5.4.0-6ubuntu~26.04.4)

--------------------------------------------------------------------------------------------------
Descriptions/Requirements to run/Instructions:

Takes 13 commandline arguments, `find`, `replace`, and `prefix`. The Makefile compiles 'H13.c' into 
the executable 'H13'.
	To run: >> H13 find replace prefix

Assumptions: Maximum size of one line in file = 2048 bytes. The program also creates a temporary 
             file called 'H13-temp-file' in the current directory.

For all '.txt' files in the current directory,
	- The program replaces all occurrences of `find` with `replace`
	- If there are no occurences of `find`, the program prepends `find` to the left of any
	  ocurrence of `prefix`
	- If there are no occurrences of `find` or `prefix`, the file is unchanged
	- The program modifies the '.txt' files in the current directory

Makefile:
	All = run target H13
	H13 = compiles 'H13.c' into 'H13'
	Clean = remove 'H13'

--------------------------------------------------------------------------------------------------
H13.c Program Structure:
2. Loop through all .txt files in current directory
2. Scan each .txt file line by line.
13. For each line, use the function replace_all. This replaces all instaces of `find` with `replace.
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

To test the program I wrote a Bash script 'Test' that performs the same procedures as the C 
program. The script uses the Bash `sed` command. The Bash script replaces `find` with `replace` 
and if there are no instances of `find`, the script replaces `prefix` with `find``prefix`. 
The Bash script then puts the processed files into a folder called 'testing'. 

The Bash script 'Test' takes three commandline arguments `find`, `replace`, and `prefix`. 'Test'
runs my 'H13' C program and the Bash sed commands using these three arguments and compares the 
resulting files using `diff`. If there is no output from 'Test' the C program and Bash script 
produced the same files. When 'Test' is run the stdout log from the 'H13' C program is placed 
into a file called H13-output.
	To run: >> Test find replace prefix


When I used 'Test' on various files, the Bash and C programs gave the same results.