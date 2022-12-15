#include <iostream>
#include "fs.h"
#include <string.h>
FS::FS()
{
    std::cout << "FS::FS()... Creating file system\n";
}

FS::~FS()
{
}

// formats the disk, i.e., creates an empty file system
int FS::format()
{
    dir_entry root[BLOCK_SIZE / 64];
    memset(fat, FAT_FREE, BLOCK_SIZE / 2 * sizeof(int16_t));
    fat[ROOT_BLOCK] = FAT_EOF;
    fat[FAT_BLOCK] = FAT_EOF;

    for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
    {
        dir_entry new_dir;
        new_dir.type = 2;
        root[i] = new_dir;
    }

    disk.write(ROOT_BLOCK, (uint8_t *)root);
    disk.write(FAT_BLOCK, (uint8_t *)fat);
    std::cout << "FS::format()\n";
    return 0;
}

// create <filepath> creates a new file on the disk, the data content is
// written on the following rows (ended with an empty row)
int FS::create(std::string filepath)
{

    // Filepath innehåller namnet, spara det i filename (kolla storleken)
    // Ge användaren en prompt för att skicka in värden
    // Avslutas när användaren skickar in en tom rad
    // ha en char array av storlek 4096. När den blir full skriv till det lediga blocket som du har hittat med disken
    // Leta efter nytt block, fortsätt ta emot värden

    // Read in root from root block
    dir_entry root[BLOCK_SIZE / 64];
    disk.read(ROOT_BLOCK, (uint8_t *)root);

    if (filepath.length() >= 56) // Check if the name is too long
    {
        return -1;
    }

    // Copies over the file name to the new dir_entry's name

    // Looks for an empty place in root directory
    int open = -1;
    for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
    {
        if (open == -1 && root[i].type == 2) // Each empty slot in root directory has a dir_entry with the type value of 2
        {
            open = i; // store index to later add our new dir_entry once all work is done
        }
        if (strcmp(root[i].file_name, filepath.c_str()) == 0) // Make sure no file has the same name
        {
            return -2;
        }
    }
    if (open == -1) // incase no empty slot is found
    {
        return -3;
    }
    std::string input;   // will store user input
    int curr_block = -1; // Will store the current block which we're writing to

    for (size_t i = 0; i < BLOCK_SIZE / 2; i++) // Will look for an empty place in the fat table
    {
        if (fat[i] == FAT_FREE) // Once an empty block is found, store index in curr_block, we will write to this block later
        {
            curr_block = i;
        }
        if (curr_block != -1) // Break when a slot is found;
            break;
    }
    if (curr_block == -1)
    {
        return -4;
    }

    // Update our direntry and fat table with the current info we have
    strcpy(root[open].file_name, filepath.c_str());
    root[open].type = 0;
    root[open].size = 0;
    root[open].first_blk = curr_block;
    root[open].access_rights = 0x04;
    fat[curr_block] = FAT_EOF;

    // receive userinput
    while (1)
    {

        std::string in; // temporary variable to store what the user wrote in this iteration
        // printf("What do you want to write in your file?\n");
        getline(std::cin, in);
        root[open].size++;
        if (in.length() <= 1 || in == "\n") // once the temporary variable receives an empty new line, break the loop
        {
            disk.write(curr_block, (uint8_t *)input.c_str()); // write all input to said block
            fat[curr_block] = FAT_EOF;

            root[open].size += input.length() - 1;
            break;
        }

        input.append(in); // appends this iterations input to the total input for our block

        while (input.length() >= BLOCK_SIZE) // need to find a new block to write to once we have more than 4096 bytes of info
        {
            std::string to_write = std::string(input.c_str(), BLOCK_SIZE); // stores the stuff we will write to the block
            disk.write(curr_block, (uint8_t *)to_write.c_str());           // writes to block
            root[open].size += to_write.length();

            input.erase(0, BLOCK_SIZE); // erases everything we wrote to the block, leaving the remaining characters beyond lenght 4096
            // find new block
            for (size_t i = 0; i < BLOCK_SIZE / 2; i++)
            {
                if (fat[i] = FAT_FREE)
                {
                    fat[curr_block] = i; // Updates fat table
                    curr_block = i;
                    fat[curr_block] = FAT_EOF;
                }
                if (curr_block != -1)
                    break;
            }
            if (curr_block == -1) // if no new block is found send error message
            {
                return -5;
            }
        }
    }
    disk.write(ROOT_BLOCK, (uint8_t *)root); // Update the disk with new information
    disk.write(FAT_BLOCK, (uint8_t *)fat);
    std::cout
        << "FS::create(" << filepath << ")\n";
    return 0;
}

// cat <filepath> reads the content of a file and prints it on the screen
int FS::cat(std::string filepath)
{
    dir_entry root[64];
    disk.read(ROOT_BLOCK, (uint8_t *)root);
    int pos = -1;

    // Look for the entry
    for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
    {
        if (strcmp(root[i].file_name, filepath.c_str()) == 0)
        {
            pos = root[i].first_blk;
            break;
        }
    }
    if (pos == -1)
    {
        return -1;
    }
    // looks until
    std::string out;
    char tmp[4096];
    {
        disk.read(pos, (uint8_t *)tmp);
        out.append(tmp);
        pos = fat[pos];
    }
    while (fat[pos] != -1)
        ;

    std::cout << "FS::cat(" + filepath + ")\n" + out << "\n";
    return 0;
}

// ls lists the content in the currect directory (files and sub-directories)
int FS::ls()
{
    dir_entry root[64];
    disk.read(ROOT_BLOCK, (uint8_t *)root);
    std::cout << "FS::ls()\n";
    std::cout << "name\t\tsize \n";
    for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
    {
        if (root[i].type != 2)
        {
            std::cout << root[i].file_name << "\t\t" << root[i].size << "\n";
        }
    }

    return 0;
}

// cp <sourcepath> <destpath> makes an exact copy of the file
// <sourcepath> to a new file <destpath>
int FS::cp(std::string sourcepath, std::string destpath)
{
    std::cout << "FS::cp(" << sourcepath << "," << destpath << ")\n";
    return 0;
}

// mv <sourcepath> <destpath> renames the file <sourcepath> to the name <destpath>,
// or moves the file <sourcepath> to the directory <destpath> (if dest is a directory)
int FS::mv(std::string sourcepath, std::string destpath)
{
    std::cout << "FS::mv(" << sourcepath << "," << destpath << ")\n";
    return 0;
}

// rm <filepath> removes / deletes the file <filepath>
int FS::rm(std::string filepath)
{
    std::cout << "FS::rm(" << filepath << ")\n";
    return 0;
}

// append <filepath1> <filepath2> appends the contents of file <filepath1> to
// the end of file <filepath2>. The file <filepath1> is unchanged.
int FS::append(std::string filepath1, std::string filepath2)
{
    std::cout << "FS::append(" << filepath1 << "," << filepath2 << ")\n";
    return 0;
}

// mkdir <dirpath> creates a new sub-directory with the name <dirpath>
// in the current directory
int FS::mkdir(std::string dirpath)
{
    std::cout << "FS::mkdir(" << dirpath << ")\n";
    return 0;
}

// cd <dirpath> changes the current (working) directory to the directory named <dirpath>
int FS::cd(std::string dirpath)
{
    std::cout << "FS::cd(" << dirpath << ")\n";
    return 0;
}

// pwd prints the full path, i.e., from the root directory, to the current
// directory, including the currect directory name
int FS::pwd()
{
    std::cout << "FS::pwd()\n";
    return 0;
}

// chmod <accessrights> <filepath> changes the access rights for the
// file <filepath> to <accessrights>.
int FS::chmod(std::string accessrights, std::string filepath)
{
    std::cout << "FS::chmod(" << accessrights << "," << filepath << ")\n";
    return 0;
}
