#include "csapp.h"
#include <pthread.h>
#define NUMTHRDS 4
#define VECLEN 100000

pthread_t callThd[NUMTHRDS];

double *array_a;
double *array_b;
double big_sum;   

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Need to protect the shared big_sum var

int veclen;
   
void *dotprod(void *arg)
{
	int i; int start; int end; // init var
	double * x; double * y;
	double mysum;
     
	/* ... */
	// This thread will only calculate from start to end
	// [0, 1, 2, ..., n]
	// arg = current thread #
	// Each thread calculates veclen segments
	// [0 to (veclen - 1), veclen to 2*veclen - 1, ...]
	// each thread starts at veclen * curretn thread #


	// arg a pointer, but the value of the pointer does not point to
	// meaningful memory
	// 
	// rather arg the pointer contains the a long variable that is the
	// current thread #, so no pointer dereference is done
	start = ((long) arg) * veclen;
	end = start + veclen;

	x = array_a; // global
	y = array_b; // global
	/* ... */

	mysum = 0;

	for (i=start; i<end ; i++)
	{
		mysum += (x[i] * y[i]);      
	}

	/* ... */
	// Protect the bigsum shared global variable
	pthread_mutex_lock(&mutex); // lock bigsum
	big_sum = big_sum + mysum;
	pthread_mutex_unlock(&mutex); // unlock

	return NULL;
	/* ... */   

}   

int main (int argc, char *argv[])
{  
	long i;
	double *a, *b;
	void *status;

	a = (double*) malloc (NUMTHRDS*VECLEN*sizeof(double));
	b = (double*) malloc (NUMTHRDS*VECLEN*sizeof(double));

	for (i=0; i<VECLEN*NUMTHRDS; i++)
	{
		a[i]=1;
		b[i]=a[i];
	}
     
	veclen = VECLEN;
	array_a = a;
	array_b = b;
	big_sum = 0;

	/* ... */
	/* create threads */
	for(i=0;i<NUMTHRDS;i++)
	{       
		/* Each thread works on a different set of data.
		The offset is specified by 'i'. The size of
		the data for each thread is indicated by VECLEN.
		*/

		// Cast i to pointer (void *)
		// i does not point to meanginful memory
		// rather i, as a pointer, contains the current thread #
		Pthread_create(&callThd[i], NULL, dotprod, (void *) i);
	}
     
	/* Wait on the other threads */
	for(i = 0; i < NUMTHRDS; i++)
	{
		Pthread_join(callThd[i], NULL);	
	}	     
	/* ... */

	printf ("Sum = %f \n", big_sum);  
	free (a);     
	free (b);    

	// Finish + cleanup
	pthread_mutex_destroy(&mutex);
	Pthread_exit(NULL);
}
