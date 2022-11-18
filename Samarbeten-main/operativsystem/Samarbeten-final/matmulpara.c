/***************************************************************************
 *
 * Partial parallelized version of Matrix-Matrix multiplication
 *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <omp.h>
#define SIZE 1024

static double a[SIZE][SIZE];
static double b[SIZE][SIZE];
static double c[SIZE][SIZE];
struct threadArgs
{
    int id;
};
static void
init_matrix(void)
{
    int i, j;

    for (i = 0; i < SIZE; i++)
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
static void
matmul_seq()
{
    int i, j, k;

    for (i = 0; i < SIZE; i++)
    {
        for (j = 0; j < SIZE; j++)
        {
            c[i][j] = 0.0;
            for (k = 0; k < SIZE; k++)
                c[i][j] = c[i][j] + a[i][k] * b[k][j];
        }
    }
}
void *
matmul_para(void *param)
{
    int i, j, k;
    // Changing the parameter into a usable datatype
    struct threadArgs *arg = (struct threadArgs *)param;
    // Storing the id in i to increase readability
    i = arg->id;
    // the forloop which iterated through rows is now instead in the main function.
    for (j = 0; j < SIZE; j++)
    {
        c[i][j] = 0.0;
        for (k = 0; k < SIZE; k++)
            c[i][j] = c[i][j] + a[i][k] * b[k][j];
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
    // Used to measure time
    double t_para = omp_get_wtime();
    init_matrix();

    struct threadArgs *args;
    args = malloc(sizeof(struct threadArgs) * SIZE);
    pthread_t handles[SIZE];
    // Assigns each row a thread which will do the multiplication
    for (size_t i = 0; i < SIZE; i++)
    {
        args[i].id = i;
        pthread_create(&handles[i], NULL, matmul_para, (void *)&args[i]);
    }

    // Once a thread has completed its work it joins back to the main thread
    for (size_t i = 0; i < SIZE; i++)
    {
        pthread_join(handles[i], NULL);
    }

    // The difference equals run time of parallelized threads
    t_para = omp_get_wtime() - t_para;

    printf("%fs for parallelized\n", t_para);
    // The sequential version is also executed to aquire the ratio in performance between the methods.
    double t_seq = omp_get_wtime();

    init_matrix();
    matmul_seq();

    t_seq = omp_get_wtime() - t_seq;
    printf("%fs for sequential\n", t_seq);

    // The ratio is printed (tends to be 2~4)
    printf("%f seq/para\n", t_seq / t_para);

    free(args);
    // print_matrix();
}
