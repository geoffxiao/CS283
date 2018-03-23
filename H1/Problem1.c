#include <stdio.h>
#include <string.h>

#define SIZE 128

int main (int argc, char* argv[])
{
	char lines_in_file[SIZE][SIZE]; // 2D array of char
	char line[SIZE]; // A line of the file
	FILE * fp = fopen(argv[1], "r"); // File to read
	if (fp == NULL) 
		return 1;

	int c = 0;
	// Read each line of the file
	while(fgets(line, SIZE, fp) != NULL)
	{
		strcpy(lines_in_file[c], line); // Copy current line into lines_in_file
		c = c + 1;
	}

	// Print each line of the file backwards
	for(int i = c - 1; i >= 0; i--)
	{
		printf("%s", lines_in_file[i]);
	}

	return 0;
}
