#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <unistd.h>
#define PERMS 0644
#define INT_MAX 60
#undef RAND_MAX
#define RAND_MAX INT_MAX
typedef struct my_msgbuf
{
   long mtype;
   int mtext;
} my_msgbuf;

int main(void)
{
   srand(time(0));
   struct my_msgbuf buf;
   int msqid;
   int len;
   key_t key;
   system("touch msgq.txt");

   if ((key = ftok("msgq.txt", 'B')) == -1)
   {
      perror("ftok");
      exit(1);
   }

   if ((msqid = msgget(key, PERMS | IPC_CREAT)) == -1)
   {
      perror("msgget");
      exit(1);
   }
   printf("message queue: ready to send messages.\n");
   sleep(7);
   buf.mtype = 1; /* we don't really care in this case */

   for (int i = 0; i < 50; i++) // A forloop that sends one message at a time
   {
      buf.mtext = rand() % (RAND_MAX + 1);
      if (msgsnd(msqid, &buf, sizeof(buf.mtext), 0) == -1) // Does the actual sending of the message
         perror("msgsnd");
      printf("sending: %d \n", buf.mtext);
      fflush(stdout);
   }
   if (msgctl(msqid, IPC_RMID, NULL) == -1)
   {
      perror("msgctl");
      exit(1);
   }
   printf("message queue: done sending messages.\n");
   return 0;
}
