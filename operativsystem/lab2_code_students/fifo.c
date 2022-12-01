#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char const *argv[])
{
    int pages;
    int size;

    if (argc == 4) //If the correct amount of arguments are parsed in, then the program continues as normal
    {
        pages = atoi(argv[1]);//Stores and converts number of pages to an int
        size = atoi(argv[2]); //Stores and converts page size to an int
    }
    else
        exit(1);


    FILE *f = fopen(argv[3], "r");
    char line[256];


    int reg[pages]; //Stores the pages currently in use
    memset(reg, -1, pages * 4); //All slots in reg are set to -1 in order to identify them as empty
    int pagefault = 0;
    int fifo_counter = 0; // Is used to store which slot is the oldest entry.

    while (fgets(line, sizeof(line), f))  //While there are rows to be grabbed this loop will continue
    {

        int row = atoi(line); //converts the reference to an int
        int found = 0;

        int index = row / size; //Gives the page in which the reference is within.
        for (size_t i = 0; i < pages; i++) //iterates page_number amount of times
        {
            if (reg[i] == -1) //If a slot is empty, increment page fault and store the page in said slot
            {
                reg[i] = index;
                pagefault++;
                found = 1;
                i = pages; //Ends the loop :)
            }
            else if (reg[i] == index) //If a slot contains the sought after page, move to next reference.
            {
                found = 1;
                i = pages;
            }
        }
        if (found == 0) //If the page is not found and all slots are taken, increment pagefault and use fifo_counter to replace the oldest entry.
        {
            reg[fifo_counter % pages] = index;
            pagefault++;
            fifo_counter++; //is incremented in order to keep track of pagefaults which occured through this manner.
        }
    }
    printf("%d pagefaults\n", pagefault);
    return 0;
}
