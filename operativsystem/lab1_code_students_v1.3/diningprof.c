enum
{
    // Different flags for professor status
    EATING,
    L_CHOPSTICK,
    R_CHOPSTICK,
    THINKING,
};
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h> /* For O_* constants */
#include <pthread.h>
#define SHMSIZE 128
#define SHM_R 0400
#define SHM_W 0200
// This int pointer is used as the shared resource between the professors
int *forks;

struct threadArgs
{
    int id;
};
// Used to prevent dataraces
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *proffessors(void *params)
{
    struct threadArgs *args = (struct threadArgs *)params;
    int id = args->id;
    // Their status is by default thinking
    int status = THINKING;

    while (1)
    {

        if (status == THINKING)
        {
            // By assigning randmax to 4, rand will give us any discrete number between 0 and 4
#undef RAND_MAX
#define RAND_MAX 4
            // by adding 1, t will always at the very least be 1 and at max 5
            unsigned long long t = rand() % (RAND_MAX + 1) + 1;
            sleep(t);
            // The left fork is defined as the professors id + 1. The fifth professor has fork 0 as its left fork hence %5
            if (forks[(id + 1) % 5] == 0)
            {
                // A lock is placed around shared memory changing operations to hinder dataraces
                pthread_mutex_lock(&lock);
                forks[(id + 1) % 5] = 1;
                pthread_mutex_unlock(&lock);
                // Updates the status of the professor
                status = L_CHOPSTICK;
                printf("%d went from thinking to getting left fork >:) \n", id + 1);
                fflush(stdout);
            }
        }
        if (status == L_CHOPSTICK)
        {
#undef RAND_MAX
#define RAND_MAX = 6
            unsigned int t = rand() % 7 + 2;
            sleep(t);
            // A professor's right fork is defined as the fork which is in position id. 0 signifies unlocked and 1 the opposite
            if (forks[id] == 0)
            {
                pthread_mutex_lock(&lock);
                forks[id] = 1;
                pthread_mutex_unlock(&lock);
                status = R_CHOPSTICK;
                printf("%d went from having left fork to getting right fork >:D %d\n", id + 1, id);
                fflush(stdout);
                // Once the professor has two forks, it eats for a random time
#undef RAND_MAX
#define RAND_MAX = 5
                unsigned int t = rand() % 6 + 5;
                sleep(t);
                printf("%d has consumed food ;)\n", id + 1);
                fflush(stdout);
                // Both forks are put down and the professor calls it a day
                pthread_mutex_lock(&lock);
                forks[id] = 0;
                forks[(id + 1) % 5] = 0;
                pthread_mutex_unlock(&lock);
                pthread_exit(NULL);
                // status = THINKING;
            }
            else // the following else statement is used to remove deadlocks,
            {    // if a professor does not manage to get ahold of a right fork after waiting for sometime,
                 // they put down the fork in frustration and go back to thinking
                pthread_mutex_lock(&lock);
                forks[(id + 1) % 5] = 0;
                pthread_mutex_unlock(&lock);
                printf("%d dropped their left fork D:\n", id + 1);
                fflush(stdout);
                status = THINKING;
            }
        }
    }
}

int main(int argc, char **argv)
{
    // generates a seed for the random numbers
    srand(time(0));
    // Allocating memory for the forks
    forks = malloc(sizeof(int) * 5);

    pthread_t *
        children; // dynamic array of child threads
    unsigned int numThreads = 5;
    // get desired # of threads
    if (argc > 1)
        numThreads = atoi(argv[1]);
    children = malloc(numThreads * sizeof(pthread_t)); // allocate array of handles
    struct threadArgs *args;
    args = malloc(numThreads * sizeof(struct threadArgs));

    for (unsigned int id = 0; id < numThreads; id++)
    {

        // create threads
        args[id].id = id;

        // printf("%d\n", forks[0]);
        pthread_create(&(children[id]),    // our handle for the child
                       NULL,               // attributes of the child
                       proffessors,        // the function it should run
                       (void *)&args[id]); // args to that function
    }
    for (unsigned int id = 0; id < numThreads; id++)
    {
        // joins back the threads once the professors have had their meal
        pthread_join(children[id], NULL);
    }
}