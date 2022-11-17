/***************************************************************************
 *
 * Fully paralellized version of Matrix-Matrix multiplication
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

// the following two functions exist to compare the parallelized to the sequential version
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

void *init_matrix_para(void *param)
{
    int i, j;
    // Changes the data type of the parameters into a useable type
    struct threadArgs *arg = (struct threadArgs *)param;
    // i is used to increase readability
    i = arg->id;
    // the forloop that exists in the parallelized version is now in main.
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

void *matmul_para(void *param)
{
    // see task 13 for an explanation
    int i, j, k;
    struct threadArgs *arg = (struct threadArgs *)param;
    i = arg->id;

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

    // Gets time since start
    double t_para = omp_get_wtime();
    struct threadArgs *args;
    // allocates enough memory for all arguments
    args = malloc(sizeof(struct threadArgs) * SIZE);
    pthread_t handles[SIZE];

    // Each row is initialised by a different thread
    for (size_t i = 0; i < SIZE; i++)
    {
        args[i].id = i;
        pthread_create(&handles[i], NULL, init_matrix_para, (void *)&args[i]);
    }
    // once a thread has initialised a row, it joins the main thread
    for (size_t i = 0; i < SIZE; i++)
    {
        pthread_join(handles[i], NULL);
    }
    // From this point onward, the program is a replica of task 13
    for (size_t i = 0; i < SIZE; i++)
    {
        args[i].id = i;
        pthread_create(&handles[i], NULL, matmul_para, (void *)&args[i]);
    }

    for (size_t i = 0; i < SIZE; i++)
    {
        pthread_join(handles[i], NULL);
    }

    t_para = omp_get_wtime() - t_para;
    printf("%fs parallelized\n", t_para);

    double t_seq = omp_get_wtime();

    init_matrix();
    matmul_seq();

    t_seq = omp_get_wtime() - t_seq;
    printf("%fs for sequential\n", t_seq);
    // Prints the quotient between the methods (tends to be 5)
    printf("%f seq/para\n", t_seq / t_para);
    free(args);
    // print_matrix();
}
