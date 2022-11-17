/***************************************************************************
 *
 * Sequential version of Matrix-Matrix multiplication
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <pthread.h>
#define SIZE 1024
#define THREADS 8
static double a[SIZE][SIZE];
static double b[SIZE][SIZE];
static double c[SIZE][SIZE];
struct threadArgs
{
    int id;
    int workload;
};
void *
init_matrix(void *params)
{
    struct threadArgs *arg = (struct threadArgs *)params;
    // i is used to increase readability
    int workloadmax = arg->workload * (arg->id + 1);
    int i, j;

    i = workloadmax - arg->workload;
    for (; i < workloadmax; i++)
        for (j = 0; j < SIZE; j++)
        {
            /* Simple initialization, which enables us to easy check
             * the correct answer. Each element in c will have the same
             * value as SIZE after the matmul operation.
             */
            a[i][j] = 1.0;
            b[i][j] = 1.0;
        }
}

void *
matmul_seq(void *params)
{
    struct threadArgs *arg = (struct threadArgs *)params;
    // i is used to increase readability
    int workloadmax = arg->workload * (arg->id + 1);
    int i, j, k;
    i = workloadmax - arg->workload;
    for (; i < workloadmax; i++)
    {
        for (j = 0; j < SIZE; j++)
        {
            c[i][j] = 0.0;
            for (k = 0; k < SIZE; k++)
                c[i][j] = c[i][j] + a[i][k] * b[k][j];
        }
    }
}

static void
print_matrix(void)
{
    int i, j;

    for (i = 0; i < SIZE; i++)
    {
        for (j = 0; j < SIZE; j++)
            printf(" %7.2f", c[i][j]);
        printf("\n");
    }
}

int main(int argc, char **argv)
{
    double t = omp_get_wtime();
    struct threadArgs *args;
    // allocates enough memory for all arguments
    args = malloc(sizeof(struct threadArgs) * THREADS);
    pthread_t handles[THREADS];
    for (size_t i = 0; i < THREADS; i++)
    {
        args[i].id = i;
        args[i].workload = (int)(SIZE / THREADS);
        pthread_create(&handles[i], NULL, init_matrix, (void *)&args[i]);
    }
    for (size_t i = 0; i < THREADS; i++)
    {
        pthread_join(handles[i], NULL);
    }
    for (size_t i = 0; i < THREADS; i++)
    {
        pthread_create(&handles[i], NULL, matmul_seq, (void *)&args[i]);
    }
    for (size_t i = 0; i < THREADS; i++)
    {
        pthread_join(handles[i], NULL);
    }
    printf("%f", omp_get_wtime() - t);
    // print_matrix();
}
