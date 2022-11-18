#include <stdio.h>
#include <unistd.h>
// Forks copy the state of the code at a given time, they are then executed independently of eachother and have no interaction.
// B is the process parent unmodified, modified version has C as the process parent
// 10991 & 10992
int main(int argc, char **argv)
{
    pid_t pid;
    unsigned i;
    unsigned niterations = 100;
    pid = fork();

    if (pid == 0)
    {
        for (i = 0; i < niterations; ++i)
            printf("A = %d, ", i);
    }
    else
    {
        printf("\n%d\n", pid);
        pid = fork();
        if (pid == 0)
        {
            for (i = 0; i < niterations; ++i)
                printf("B = %d, ", i);
        }
        else
        {
            for (i = 0; i < niterations; ++i)
                printf("C = %d, ", i);
            printf("\n%d\n", pid);
        }
    }
    printf("\n");
}
