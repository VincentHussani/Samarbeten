#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h> /* For O_* constants */
#define SHMSIZE 128
#define SHM_R 0400
#define SHM_W 0200

// The consumer tends to bottleneck the producer, by using 10 semaphores,
// the producer can quickly fill all slots whilst the consumer is still reading and then it is bottlenecked waiting for a semaphore to tick back to 1

const char *semName1 = "my_semap";
const char *semName2 = "my_semaph";
int i = 0;
int main(int argc, char **argv)
{
    srand(time(0));
    sem_unlink(semName1);
    sem_unlink(semName2);
    // Our two semaphores, the initalising values are in such a manner that sem1 is
    // reduced to 8 whilst semid2 is increased to 2 during the first call in parent thread
    sem_t *sem_id1 = sem_open(semName1, O_CREAT, O_RDWR, 9);
    sem_t *sem_id2 = sem_open(semName2, O_CREAT, O_RDWR, 1);
    int status;
    struct shm_struct
    {
        // one buffer containing the status of a given position, whilst the other contains numerical values.
        int buffer[10];
        unsigned empty[10];
    };

    volatile struct shm_struct *shmp = NULL;
    char *addr = NULL;
    pid_t pid = -1;
    int var1 = 0, var2 = 0, shmid = -1;
    struct shmid_ds *shm_buf;

    /* allocate a chunk of shared memory */
    shmid = shmget(IPC_PRIVATE, SHMSIZE, IPC_CREAT | SHM_R | SHM_W);
    shmp = (struct shm_struct *)shmat(shmid, addr, 0);

    pid = fork();
    if (pid)
    {
#undef RAND_MAX
#define RAND_MAX 3
        int i = 0;
        while (var1 < 100)
        {
            var1++;
            int found = 0;
            // The following for loop acts as a busy wait until a slot opens
            while (found == 0)
            {
                sem_wait(sem_id1);
                // i%10 makes accessing the buffer circular, if empty is 0 then it has nothing
                if (shmp->empty[i % 10] != 1)
                {

                    // Decrements sem_id1
                    shmp->buffer[i % 10] = var1;
                    shmp->empty[i % 10] = 1;
                    // increments sem_id2
                    sem_post(sem_id2);
                    printf("Sending %d\n", var1);
                    fflush(stdout);
                    // Used to sleep a random value between 0.2 and 0.5 seconds
                    int t = 10000 * (rand() % (RAND_MAX + 1) + 2);
                    i = i % 10;
                    usleep(t);
                    // once an empty slot is found, the parent process proceeds to the next number
                    found = 1;
                }
                else
                {
                    sem_post(sem_id1);
                }

                i++;
            }

            /* write to shmem */
        }
        // Closes/Detaches the shared memory and semaphores upon finishing its sequence
        sem_close(sem_id1);
        sem_close(sem_id2);
        wait(&status);
        sem_unlink(semName1);
        sem_unlink(semName2);
        shmdt(addr);
        shmctl(shmid, IPC_RMID, shm_buf);
    }
    else
    {
#undef RAND_MAX
#define RAND_MAX 18
        while (var2 < 100)
        {
            sem_wait(sem_id2);
            // Once a value has been found it reads and prints said value
            if (shmp->empty[i % 10])
            {
                // the new value is stored in var2
                var2 = shmp->buffer[i % 10];
                shmp->empty[i % 10] = 0;
                sem_post(sem_id1);

                printf("Received %d\n", var2);
                fflush(stdout);
                i = i % 10;
                int t = 10000 * (rand() % (RAND_MAX + 1) + 2);
                usleep(t);
            }
            else
            {
                sem_post(sem_id2);
            }
            i++;
        }
        sem_close(sem_id1);
        sem_close(sem_id2);
        shmdt(addr);
        shmctl(shmid, IPC_RMID, shm_buf);
    }
}