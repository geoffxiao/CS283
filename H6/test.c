#include "csapp.h"

void *thread(void *vargp);

int main(int argc, char** argv)
{
	pthread_t tid;
	
	Pthread_create(&tid, NULL, thread, NULL);

	// Pthread_join(tid, NULL);
	pthread_exit(NULL);
	exit(0);
}

void *thread(void *vargp)
{
	Sleep(1);
	printf("Hello World!\n");
	return NULL;
}
