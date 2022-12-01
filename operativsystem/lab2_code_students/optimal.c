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


    int num_rows = 0; //To make the code more general-use, the program counts all rows to create an array with an appropiate size
    while (fgets(line, sizeof(line), f))
    {
        num_rows++;
    }

    rewind(f); //resets where f is pointing to

    int ref[num_rows]; //will contain all references
    int slot = 0;
    while (fgets(line, sizeof(line), f))
    {
        ref[slot] = atoi(line); //adds all references to ref
        slot++;
    }

    int reg[pages];
    memset(reg, -1, pages * 4);
    int pagefaults = 0;

    int distance[pages]; //used to see when a slot will be accessed in the future.
    memset(distance, -1, pages * 4); //Each row is set to -1 which indicates that it will never be used again.

    for (size_t i = 0; i < num_rows; i++) //for loop iterates as many times as there are references
    {
        int row = ref[i];
        int found = 0;
        int curr_page = (int)(row / size); //gets the reference's page

        for (size_t j = 0; j < pages; j++) //Iterates through the slots
        {
            if (reg[j] == -1) //If a slot is empty, occupy it and increment pagefault.
            {
                reg[j] = curr_page;
                pagefaults++;
                found = 1;
                j = pages; //stops the innermost loop
            }
            else if (reg[j] == curr_page)
            {
                found = 1;
                j = pages;
            }
        }
        if (found == 0)
        {
            for (size_t j = i; j < num_rows; j++) //iterates through all references, starting from the current reference's index.
            {

                for (size_t k = 0; k < pages; k++)//iterates through our currently used pages
                {
                    if (distance[k] == -1 && reg[k] == (int)(ref[j] / size)) //Activates at the first occurence of a match between our currently used pages and the pages ahead of us
                    {
                        distance[k] = i-j; //number of refs ahead of us until said page is accessed.
                        k = pages;
                    }
                }
            }
            int i_remove = 0;
            int highest = distance[0]; //the slot with the highest distance value will be replaced (unless any slot is never accessed again)
            for (size_t k = 0; k < pages; k++)
            {
                if (distance[k] == -1)//If a slot is never accessed, then it is replaced
                {
                    i_remove = k;
                    k = pages;
                }
                else if (highest < distance[k])//Used to find the highest number of refs between now and next access point
                {
                    i_remove = k;
                    highest = distance[k];
                }
            }
            reg[i_remove] = curr_page; //replaces the page
            memset(distance, -1, 4 * pages); //Resets distance for the future
            pagefaults++;
        }
    }
    printf("%d pagefaults\n", pagefaults);
    return 0;
}
