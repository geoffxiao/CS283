/*

Geoffrey Xiao
CS 283 H1
Problem 2. Write a C program to convert a decimal number to binary in 32 bits.
All 32 bits should be printed even if fewer bits are required to represent the number. The
number should be specified as a command-line argument.

*/

#include <stdio.h>
#include <stdlib.h>

#define SIZE 32

// power(base, exp) = base^exp
int power(int base, int exp);

int main(int argc, char* argv[])
{
	int num = atoi(argv[1]); // Convert to an integer
	int curr = SIZE - 1; // Max bit - 1, curr = current bit

	while(curr >= 0) // Go through all 32 bits
	{
		if((num - power(2,curr)) >= 0)
		{
			num = num - power(2,curr);
			printf("1"); // 1 at this bit
		}
		else
		{
			printf("0"); // 0 at this bit
		}

		curr = curr - 1;
	}

	printf("\n");
	return 0;
}

// power(base, exp) = base^exp
int power(int base, int exp)
{
	int out = 1;
	for(int i = 0; i < exp; i++)
	{
		out = out * base;
	}	
	return out;
}
