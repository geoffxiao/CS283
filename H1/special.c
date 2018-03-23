/*

Geoffrey Xiao
CS 283 H1
Problem 1. Write a C program to print the contents of a text file line-by-line but
backwards so that the last line is printed first. The filename should be specified as a
command-line argument.

*/

#include <stdio.h>

int main (int argc, char* argv[])
{
	FILE * fp = fopen(argv[1], "r"); // File to read = command line arg
	if (fp == NULL) 
		return 1;

	// First byte in file
	fseek(fp, 0, SEEK_SET);
	long int beginning = ftell(fp);

	// Move file position to one char before EOF
	fseek(fp, -1, SEEK_END);

	// Some files have \n before EOF
	char curr_char = fgetc(fp);
	long int char_count; // Number of chars processed before seeing a \n
	if(curr_char == '\n') // If \n before EOF, ignore the \n
	{
		fseek(fp, -2, SEEK_CUR); // Move pos to before the \n
		char_count = 1;
	}
	else // no \n before EOF
	{
		char_count = 0;
	}
	long int curr = ftell(fp); // curr pos in file
	
	/*
		1. Start at EOF. Some files end in \n EOF.
		2. Read chars backwards until we see \n. Move file position curr
		backwards while we read. Increment char_count on each read backwards.
		3. See \n. Then print the next char_count chars from the curr file position.
		4. Move curr file position back char_count+1 chars.
		5. Repeat steps 2 and 4 until beginning of file.
		6. Print the first line.
	 */


	// Keep reading until we reach beginning of file
	while(curr >= beginning)
	{
		curr_char = fgetc(fp); // read curr char
		// curr++ b/c of fgetc

		if(curr_char == '\n') // EOL
		{
			// print char_count chars starting from curr pos
			// curr pos is actually after EOL b/c fgetc causes curr++
			for(int i = 0; i < char_count; i++)
			{
				printf("%c", fgetc(fp));
			}
			fseek(fp, -char_count, SEEK_CUR); // Move back file position
			char_count = 0; // reset char counter
		}
		
		fseek(fp, -2, SEEK_CUR);
		// move back file position -2

		curr = curr - 1; // Keep track of curr file pos
		char_count++; // char counter
	}

	// Now print the first line
	fseek(fp, 0, SEEK_SET);
	for(int i = 0; i < char_count; i++)
	{
		printf("%c", fgetc(fp));
	}

	return 0;
}