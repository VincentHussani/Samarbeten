#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct threadArgs
{
	unsigned int id;
	unsigned int numThreads;
	unsigned int squaredId;
};

void *child(void *params)
{
	// unpacking the args into a usable datatype
	struct threadArgs *args = (struct threadArgs *)params;
	unsigned int childID = args->id;
	unsigned int numThreads = args->numThreads;
	// The squaring of the ID happens here (We added 1 to the childID to make it base 1 instead of 0)
	args->squaredId = (childID + 1) * (childID + 1);
	printf("Greetings from child #%u of %u\n", childID + 1, numThreads);
}

int main(int argc, char **argv)
{
	pthread_t *children;	 // dynamic array of child threads
	struct threadArgs *args; // argument buffer
	unsigned int numThreads = 4;
	// get desired # of threads
	if (argc > 1)
		numThreads = atoi(argv[1]);
	children = malloc(numThreads * sizeof(pthread_t));	   // allocate array of handles
	args = malloc(numThreads * sizeof(struct threadArgs)); // args vector
	for (unsigned int id = 0; id < numThreads; id++)
	{
		// create threads
		args[id].id = id;
		args[id].numThreads = numThreads;
		pthread_create(&(children[id]),	   // our handle for the child
					   NULL,			   // attributes of the child
					   child,			   // the function it should run
					   (void *)&args[id]); // args to that function
	}
	printf("I am the parent (main) thread.\n");
	for (unsigned int id = 0; id < numThreads; id++)
	{
		pthread_join(children[id], NULL);
		// Once the children have joined back to the main thread, their squaredID is printed
		printf("%d\n", args[id].squaredId);
	}
	free(args);		// deallocate args vector
	free(children); // deallocate array
	return 0;
}
