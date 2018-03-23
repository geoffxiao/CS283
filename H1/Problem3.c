/*

Geoffrey Xiao
CS 283 H1
Problem 3. Write a C program to find out how many bits a variable of type long
int is. Your program should not call any system functions (not even sizeof) and it should
not include any .h files. (In other words, write your own code).

*/

int main()
{
	long int ptr[2]; // Array of long variables

	int size_long = (char*)&ptr[1] - (char*)&ptr[0]; // Size of a long

	printf("Bytes in a long: %d\n", size_long); // Print results
	printf("Bytes in a long using sizeof: %ld\n", sizeof(long int)); // sizeof results

	return 0;
}
