#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define PERMS 0644
struct my_msgbuf
{
   long mtype;
   int num;
   char bruh[200]
};

int main(void)
{
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
   printf("Enter lines of text, ^D to quit:\n");
   buf.num = 5;
   buf.mtype = 1; /* we don't really care in this case */
   len = sizeof(int);
   while (fgets(buf.bruh, sizeof buf.bruh, stdin) != NULL)
   {
      /* remove newline at end, if it exists */
      if (msgsnd(msqid, &buf.num, len, 0) == -1) /* +1 for '\0' */
         perror("msgsnd");
   }
   if (msgsnd(msqid, &buf.num, len, 0) == -1) /* +1 for '\0' */
      perror("msgsnd");

   if (msgctl(msqid, IPC_RMID, NULL) == -1)
   {
      perror("msgctl");
      exit(1);
   }
   printf("message queue: done sending messages.\n");
   return 0;
}
