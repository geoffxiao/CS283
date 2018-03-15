#include <pthread.h>
#include "csapp.h"

#define NUM_THREADS 8

char *messages[NUM_THREADS];

/* ... */
// Create structure to contain arguments
struct arguments
{
	int sum;
	char * msg;
	pthread_t * tid;
};

typedef struct arguments arg_struct;	
/* ... */

void *PrintHello(void *threadarg)
{
	int taskid, sum;
	char *hello_msg;     

	/* ... */
	// Get arguments from structure
	arg_struct temp = *((arg_struct *) threadarg);
	
	sum = temp.sum;
	hello_msg = temp.msg;
	taskid = temp.tid;
	/* ... */

	printf("Thread %d %s Sum=%d\n", taskid, hello_msg, sum);     

	pthread_exit(NULL);

}
   
int main(int argc, char *argv[])    
{
	pthread_t threads[NUM_THREADS];
	int rc, t, sum;
	
	sum=0;
	messages[0] = "Hello-0";
	messages[1] = "Hello-1";
	messages[2] = "Hello-2";
	messages[3] = "Hello-3";
	messages[4] = "Hello-4";
	messages[5] = "Hello-5";
	messages[6] = "Hello-6";
	messages[7] = "Hello-7";
   
	for(t = 0; t < NUM_THREADS; t++)
	{
		sum = sum + t;      

		/* ... */
		// Package arguments into dynamically allocated structure
		arg_struct * arg = (arg_struct *) calloc(1, sizeof(arg_struct));
		arg->sum = sum;
		arg->msg = malloc( (strlen(messages[t]) + 1) * sizeof(char) );
		strcpy(arg->msg, messages[t]);
		arg->tid = &threads[t];

		printf("Creating thread %d\n", t);      
		Pthread_create(&threads[t], NULL, PrintHello, (void*) arg); // Create thread
		Pthread_join(threads[t], NULL); // Wait	
		
		// Free heap
		free(arg->msg);
		free(arg);
		/* ... */

	}
	pthread_exit(NULL);
}
