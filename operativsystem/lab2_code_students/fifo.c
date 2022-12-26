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

    int reg[pages];
    memset(reg, -1, pages * 4);
    int pagefault = 0;
    int fifo_counter = 0;
    while (fgets(line, sizeof(line), f))
    {
        int row = atoi(line);
        int found = 0;
        int index = row / size;
        for (size_t i = 0; i < pages; i++)
        {
            if (reg[i] == -1)
            {
                reg[i] = index;
                pagefault++;
                found = 1;
                i = pages;
            }
            else if (reg[i] == index)
            {
                found = 1;
                i = pages;
            }
        }
        if (found == 0)
        {
            reg[fifo_counter % pages] = index;
            pagefault++;
            fifo_counter++;
        }
    }
    printf("%d pagefaults\n", pagefault);
    return 0;
}