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

const char *semName1 = "my_semap";
const char *semName2 = "my_semaph";
int i = 0;
int main(int argc, char **argv)
{
    sem_t *sem_id1 = sem_open(semName1, O_CREAT, O_RDWR, 10);
    sem_t *sem_id2 = sem_open(semName2, O_CREAT, O_RDWR, 9);
    int status;
    struct shm_struct
    {
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
#define RAND_MAX 3;
        int i = 0;
        while (var1 < 100)
        {
            var1++;
            int found = 0;
            while (found == 0)
            {
                if (shmp->empty[i % 10] != 1)
                {
                    srand(time(0));

                    printf("Sending %d\n", var1);
                    fflush(stdout);
                    sem_wait(sem_id1);
                    shmp->buffer[i % 10] = var1;
                    shmp->empty[i % 10] = 1;
                    sem_post(sem_id2);

                    int t = 10000 * (rand() % 4 + 2);
                    i = i % 10;
                    usleep(t);
                    found = 1;
                }

                i++;
            }

            /* write to shmem */
        }
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
#define RAND_MAX = 18;
        while (var2 < 100)
        {

            int found = 0;
            while (found == 0)
            {
                if (shmp->empty[i % 10])
                {
                    srand(time(0));

                    sem_wait(sem_id2);
                    var2 = shmp->buffer[i % 10];
                    shmp->empty[i % 10] = 0;
                    sem_post(sem_id1);

                    printf("Received %d\n", var2);
                    fflush(stdout);
                    i = i % 10;
                    int t = 10000 * (rand() % 19 + 2);
                    usleep(t);
                    found = 1;
                }
                i++;
            }
        }
        sem_close(sem_id1);
        sem_close(sem_id2);
        shmdt(addr);
        shmctl(shmid, IPC_RMID, shm_buf);
    }
}
