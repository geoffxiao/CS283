#include "csapp.h"

void *thread(void *vargp);

int main(int argc, char** argv)
{
	int n = atoi(argv[1]); // number of threads to create

	pthread_t tid[n]; // array of thread IDs

	// Create n threads
	for(int i = 0; i < n; i++)
		Pthread_create(&tid[i], NULL, thread, NULL);

	// Join n threads
	for(int i = 0; i < n; i++)
		Pthread_join(tid[i], NULL);

	exit(0);
}

// Function the thread exceutes
void *thread(void *vargp)
{
	printf("Hello!\n");
	return NULL;
}
