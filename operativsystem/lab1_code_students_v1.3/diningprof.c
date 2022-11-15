enum
{
    EATING,
    L_CHOPSTICK,
    R_CHOPSTICK,
    THINKING
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

struct threadArgs
{
    int fork[5];
    int id;
};
void *proffessors(void *params)
{
    struct threadArgs *args = (struct threadArgs *)params;
    unsigned int *fork = args->fork;
    volatile int id = args->id;
    int status = THINKING;
    srand((id + 3) * time(0));
    while (1)
    {
        if (status == THINKING)
        {
#undef RAND_MAX
#define RAND_MAX = 4
            unsigned int t = rand() % 3 + 1;
            sleep(t);
            if (fork[(id + 1) % 5] == 0)
            {
                fork[(id + 1) % 5] = 1;
                status = L_CHOPSTICK;
                printf("%d went from thinking to getting left fork\n", id + 1);
                fflush(stdout);
            }
        }
        if (status == L_CHOPSTICK)
        {
#undef RAND_MAX
#define RAND_MAX = 6
            unsigned int t = rand() % 5 + 2;
            printf("%d\n", fork[2]);
            sleep(t);
            if (fork[(id - 1) % 5] == 0)
            {
                fork[(id - 1) % 5] = 1;
                status = R_CHOPSTICK;
                printf("%d went from having left fork to getting right fork\n", id + 1);
                fflush(stdout);
#undef RAND_MAX
#define RAND_MAX = 5
                unsigned int t = rand() % 4 + 5;
                sleep(t);
                printf("%d has consumed food ;)\n", id + 1);
                fflush(stdout);
                fork[(id - 1) % 5] = 0;
                fork[(id + 1) % 5] = 0;
                pthread_exit(NULL);
                status = THINKING;
            }
        }
        printf("%d\n", fork[2]);
    }
}

int main(int argc, char **argv)
{
    int forks[5] = {0, 0, 0, 0, 0};

    pthread_t *children; // dynamic array of child threads
    unsigned int numThreads = 5;
    // get desired # of threads
    if (argc > 1)
        numThreads = atoi(argv[1]);
    children = malloc(numThreads * sizeof(pthread_t)); // allocate array of handles
    for (unsigned int id = 0; id < numThreads; id++)
    {
        struct threadArgs args;
        // create threads
        args->id = id;
        *args->fork = *forks;
        printf("%d\n", forks[0]);
        pthread_create(&(children[id]), // our handle for the child
                       NULL,            // attributes of the child
                       proffessors,     // the function it should run
                       (void *)&args);  // args to that function
    }
    for (unsigned int id = 0; id < numThreads; id++)
    {
        pthread_join(children[id], NULL);
    }
}
