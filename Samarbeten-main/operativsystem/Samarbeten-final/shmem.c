#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#define SHMSIZE 128
#define SHM_R 0400
#define SHM_W 0200

int main(int argc, char **argv)
{
	struct shm_struct
	{
		int buffer[10];
		unsigned empty;
	};
	volatile struct shm_struct *shmp = NULL;
	char *addr = NULL;
	pid_t pid = -1;
	int var1 = 0, var2 = 0, shmid = -1;
	struct shmid_ds *shm_buf;

	/* allocate a chunk of shared memory */
	shmid = shmget(IPC_PRIVATE, SHMSIZE, IPC_CREAT | SHM_R | SHM_W);
	shmp = (struct shm_struct *)shmat(shmid, addr, 0);
	shmp->empty = 0;
	pid = fork();
	if (pid != 0)
	{
		/* here's the parent, acting as producer */
		while (var1 < 100)
		{
			/* write to shmem */
			var1++;
			for (size_t i = var1; i < var1 + 10; i++)
			{
				shmp->buffer[i] = var1;
				shmp->empty = i;
				printf("Sending %d\n", var1);
				fflush(stdout);
#define RAND_MAX 4;
				float t = rand() + 1;
				sleep(t / 10);
			}

			while (shmp->empty == 1)
				; /* busy wait until the buffer is empty */
		}
		shmdt(addr);
		shmctl(shmid, IPC_RMID, shm_buf);
	}
	else
	{
		/* here's the child, acting as consumer */
		while (var2 < 100)
		{
			/* read from shmem */
			while (shmp->empty == 0)
				; /* busy wait until there is something */
			var2 = shmp->buffer;
			shmp->empty = 0;
			printf("Received %d\n", var2);
			fflush(stdout);
		}
		shmdt(addr);
		shmctl(shmid, IPC_RMID, shm_buf);
	}
}
