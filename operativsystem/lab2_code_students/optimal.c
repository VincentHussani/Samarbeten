#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char const *argv[])

{
    int pages;
    int size;
    if (argc != 1)
    {
        char *page_tmp = argv[1];
        pages = atoi(page_tmp);

        char *size_tmp = argv[2];
        size = atoi(size_tmp);
    }

    char *file = argv[3];
    FILE *f = fopen(file, "r");
    char line[256];

    int num_rows = 0;
    while (fgets(line, sizeof(line), f))
    {
        num_rows++;
    }

    rewind(f);
    int ref[num_rows];
    int slot = 0;

    while (fgets(line, sizeof(line), f))
    {
        ref[slot] = atoi(line);
        slot++;
    }

    int reg[pages];
    memset(reg, -1, pages * 4);
    int pagefaults = 0;

    int distance[pages];
    memset(distance, -1, pages * 4);

    for (size_t i = 0; i < num_rows; i++)
    {
        int row = ref[i];
        int found = 0;
        int index = (int)(row / size);

        for (size_t j = 0; j < pages; j++)
        {
            if (reg[j] == -1)
            {
                reg[j] = index;
                pagefaults++;
                found = 1;
                j = pages;
            }
            else if (reg[j] == index)
            {
                found = 1;
                j = pages;
            }
        }
        if (found == 0)
        {
            int counter = 0;
            for (size_t j = i; j < num_rows; j++)
            {

                for (size_t k = 0; k < pages; k++)
                {
                    if (distance[k] == -1 && reg[k] == (int)(ref[j] / size))
                    {
                        distance[k] = counter;
                        k = pages;
                    }
                }
                counter++;
            }
            int i_remove = 0;
            int highest = distance[0];
            for (size_t k = 0; k < pages; k++)
            {
                if (distance[k] == -1)
                {
                    i_remove = k;
                    k = pages;
                }
                else if (highest < distance[k])
                {
                    i_remove = k;
                    highest = distance[k];
                }
            }
            reg[i_remove] = index;
            memset(distance, -1, 4 * pages);
            pagefaults++;
        }
    }
    printf("%d pagefaults\n", pagefaults);
    return 0;
}