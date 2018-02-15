#include "csapp.h"

int main()
{

	int i;
	for(i = 3; i > 0; i--) // i = 3, 2, 1
	{
		Fork();
	}
	printf("Example\n");
	exit(0);

}
