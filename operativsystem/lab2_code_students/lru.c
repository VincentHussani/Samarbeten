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


    int reg[pages]; //stores all curently used pages
    memset(reg, -1, pages * 4); //-1 = empty

    int lru[pages]; //Is used to keep track of how many references were read since a coressponding slot was last accessed
    memset(lru, 0, 4 * pages); //get rid of potential junk

    int count = 0; //Count is used to turn the while-loop into a for-loop of sorts.

    int pagefault = 0;
    while (fgets(line, sizeof(line), f)) //While there are references to grab, this while loop will continue.
    {
        int row = atoi(line); //Converts the references to an int
        int found = 0;
        int lowest = count; //Lowest is used to identify which slot has been accessed the least recently
        int lowest_i = 0; //has the index of said slot
        int index = (int)(row / size); //Gives the page of our reference
        for (size_t i = 0; i < pages; i++)
        {
            if (reg[i] == -1) //If a slot is empty, occupy it and increment pagefault
            {
                reg[i] = index;
                pagefault++;
                found = 1;
                lru[i] = count; //updates the slot's last time accessed value
                i = pages;
            }
            else if (reg[i] == index)
            {
                found = 1;
                lru[i] = count; //Updates the slot's last time accessed value.
                i = pages;
            }
            if (lowest > lru[i]) //Done to keep track of the least recently used incase a pagefault will occur
            {
                lowest = lru[i];
                lowest_i = i;
            }
        }
        if (found == 0) //If no current slot is usable, replace the least recently used row
        {
            reg[lowest_i] = index;
            lru[lowest_i] = count;
            pagefault++;
        }
        count++; //Incremented each time to store usage data.
    }

    printf("%d pagefaults\n", pagefault);
    return 0;
}
