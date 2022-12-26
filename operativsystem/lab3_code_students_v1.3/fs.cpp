#include <iostream>
#include "fs.h"
#include <string.h>
int working_block = 0;
FS::FS()
{
    std::cout << "FS::FS()... Creating file system\n";
}

FS::~FS()
{
}
path_info FS::getpath(std::string filepath)
{

    path_info path;

    path.block = -1;
    int dest_block;
    std::string file_name;
    dir_entry dest_dir[BLOCK_SIZE / 64];
    std::string filepath_arr = filepath;
    int index = 0;
    int subdir_block = 0;
    if (filepath[0] == '/') // relative
    {
        disk.read(ROOT_BLOCK, (u_int8_t *)dest_dir);
        subdir_block = ROOT_BLOCK;
        filepath_arr.erase(0, 1);
        // HELP std::cout << "Relative path\n";

        // std::cout << filepath_arr << " HELLO ???\n";
    }
    else // absolute path
    {
        // HELP std::cout << filepath_arr << " is an absolute path\n";
        // std::cout << "Reading from block: " << working_block << "\n";
        subdir_block = working_block;
        disk.read(working_block, (u_int8_t *)dest_dir);
    }

    while (1)
    {
        std::string subdir_name;
        size_t subdir_index = filepath_arr.find('/');
        if (subdir_index == std::string::npos)
        {
            // HELP std::cout << "WE MADE IT for" << filepath << "\n";
            file_name = filepath_arr;
            dest_block = subdir_block;
            break;
        }
        subdir_name = filepath_arr.substr(0, subdir_index);
        // HELP std::cout << subdir_name << " is the name of our current dir for: " << filepath << "\n";
        filepath_arr.erase(0, subdir_index + 1);
        char found = 0;

        for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
        {
            // std::cout << "looking for: \n"
            //<< subdir_name;
            if (strcmp(subdir_name.c_str(), dest_dir[i].file_name) == 0 && dest_dir[i].type == 1)
            {
                found = 1;
                subdir_block = dest_dir[i].first_blk;
                index = i;
                disk.read(subdir_block, (uint8_t *)dest_dir);
                break;
            }
        }
        if (found == 0)
        {
            return path;
        }
    }
    path.block = dest_block;
    path.file_name = file_name;
    // HELP std::cout << file_name << " in " << dest_block << '\n';
    if (dest_block != 0)
    {
        path.block_type = dest_dir[index].type;
    }
    else
        path.block_type = 1;
    // HELP std::cout << file_name << " in " << dest_block << '\n';
    return path;
}
void FS::updatesize(uint32_t size, int originblock)
{
    dir_entry origin_dir[64];
    disk.read(originblock, (uint8_t *)origin_dir);
    if (originblock != 0)
    {
        int parent_blk = origin_dir[0].first_blk;
        dir_entry parent_dir[BLOCK_SIZE / 64];
        while (parent_dir[0].first_blk != 0)
        {
            disk.read(parent_blk, (uint8_t *)parent_dir);
            for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
            {
                if (parent_dir[i].first_blk == parent_blk)
                {

                    parent_dir[i].size += size;
                    disk.write(parent_blk, (uint8_t *)parent_dir);
                    break;
                }
            }
            parent_blk = parent_dir[0].first_blk;
        }
    }
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
        root[i].type = 2;
    }

    disk.write(ROOT_BLOCK, (uint8_t *)root);
    disk.write(FAT_BLOCK, (uint8_t *)fat);
    std::cout
        << "FS::format()\n";
    return 0;
}
// FINISHED
//  create <filepath> creates a new file on the disk, the data content is
//  written on the following rows (ended with an empty row)
int FS::create(std::string filepath)
{
    // return 0;
    //  Filepath innehåller namnet, spara det i filename (kolla storleken)
    //  Ge användaren en prompt för att skicka in värden
    //  Avslutas när användaren skickar in en tom rad
    //  ha en char array av storlek 4096. När den blir full skriv till det lediga blocket som du har hittat med disken
    //  Leta efter nytt block, fortsätt ta emot värden

    int dest_block;
    std::string file_name;
    dir_entry dest_dir[BLOCK_SIZE / 64];
    path_info path_info = getpath(filepath);
    if (path_info.block == -1)
    {
        return -1;
    }
    file_name = path_info.file_name;
    dest_block = path_info.block;
    if (file_name.length() >= 56) // Check if the name is too long
    {
        return -1;
    }
    disk.read(dest_block, (uint8_t *)dest_dir);
    // Copies over the file name to the new dir_entry's name

    // Looks for an empty place in root directory
    int open = -1;
    for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
    {

        if (open == -1 && dest_dir[i].type == 2) // Each empty slot in root directory has a dir_entry with the type value of 2
        {
            // printf("FOUND SLOT\n");
            open = i;
            break; // store index to later add our new dir_entry once all work is done
        }
        if (strcmp(dest_dir[i].file_name, file_name.c_str()) == 0) // Make sure no file has the same name
        {
            return -2;
        }
    }
    if (open == -1) // incase no empty slot is found
    {
        printf("Did not find a slot \n");
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
    strcpy(dest_dir[open].file_name, file_name.c_str());
    dest_dir[open].type = 0;
    dest_dir[open].size = 0;
    dest_dir[open].first_blk = curr_block;
    dest_dir[open].access_rights = READ | WRITE;
    fat[curr_block] = FAT_EOF;

    // receive userinput
    while (1)
    {

        std::string in; // temporary variable to store what the user wrote in this iteration
        // printf("What do you want to write in your file?\n");
        getline(std::cin, in);
        dest_dir[open].size++;
        if (in.length() <= 1 || in == "\n") // once the temporary variable receives an empty new line, break the loop
        {
            input.append(in);
            disk.write(curr_block, (uint8_t *)input.c_str()); // write all input to said block
            fat[curr_block] = FAT_EOF;

            dest_dir[open].size += input.length() - 1;
            break;
        }

        input.append(in); // appends this iterations input to the total input for our block

        while (input.length() >= BLOCK_SIZE) // need to find a new block to write to once we have more than 4096 bytes of info
        {
            std::string to_write = std::string(input.c_str(), BLOCK_SIZE); // stores the stuff we will write to the block
            disk.write(curr_block, (uint8_t *)to_write.c_str());           // writes to block
            dest_dir[open].size += to_write.length();
            int old_block = curr_block;
            input.erase(0, BLOCK_SIZE); // erases everything we wrote to the block, leaving the remaining characters beyond lenght 4096
            // find new block
            for (size_t i = 0; i < BLOCK_SIZE / 2; i++)
            {
                if (fat[i] == FAT_FREE)
                {
                    fat[curr_block] = i; // Updates fat table
                    curr_block = i;
                    fat[curr_block] = FAT_EOF;
                }
                if (curr_block != old_block)
                    break;
            }
            if (curr_block == old_block) // if no new block is found send error message
            {
                return -5;
            }
        }
    }
    // Update the directory size by reading in parent directory and looking for the same working block;
    // updatesize(dest_dir[open].size, dest_block);
    updatesize(dest_dir[open].size, dest_block);
    disk.write(dest_block, (uint8_t *)dest_dir); // Update the disk with new information
    disk.write(FAT_BLOCK, (uint8_t *)fat);
    std::cout
        << "FS::create(" << filepath << ")\n";
    return 0;
}

// FINISHED

// cat <filepath> reads the content of a file and prints it on the screen
int FS::cat(std::string filepath)
{
    int dest_block;
    std::string file_name;
    dir_entry dest_dir[BLOCK_SIZE / 64];
    path_info path_info = getpath(filepath);
    if (path_info.block == -1)
    {
        return -1;
    }
    file_name = path_info.file_name;
    dest_block = path_info.block;
    disk.read(dest_block, (uint8_t *)dest_dir);
    int pos = -1;

    // Look for the entry
    for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
    {
        if (strcmp(dest_dir[i].file_name, file_name.c_str()) == 0)
        {
            pos = dest_dir[i].first_blk;
            break;
        }
    }
    if (pos == -1 || dest_dir[pos].type == 1)
    {
        return -2;
    }
    if ((dest_dir[pos].access_rights & READ) != READ)
    {
        return -3;
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
    // std::cout << "\nLS\n"
    //           << "reading from " << working_block << "\n";

    dir_entry root[64];
    disk.read(working_block, (uint8_t *)root);
    // printf("reading from block %d\n\n", working_block);
    std::cout << "FS::ls()\n";
    std::cout << "name\t type\t accessrights\t size \n";
    for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
    {
        if ((root[i].type == 1) || (root[i].type == 0) && (root[i].first_blk != -1))
        {
            std::string filetype;
            std::string rights = "---";
            if (root[i].type == 1)
                filetype = "dir";
            if (root[i].type == 0)
                filetype = "file";
            if ((root[i].access_rights & READ) == READ)
            {
                rights[0] = 'r';
                rights[2] = ' ';
            }

            if ((root[i].access_rights & WRITE) == WRITE)
            {
                rights[1] = 'w';
                rights[2] = '-';
            }
            if ((root[i].access_rights & EXECUTE) == EXECUTE)
            {
                rights[1] = 'x';
            }

            std::cout << root[i].file_name << "\t" << filetype << "\t" << rights << "\t" << root[i].size << "\t\n";
        }
    }

    return 0;
}

// FINISHED
//  cp <sourcepath> <destpath> makes an exact copy of the file
//  <sourcepath> to a new file <destpath>
int FS::cp(std::string sourcepath, std::string destpath)
{
    // return 0;
    int src_block;
    std::string src_name;
    dir_entry src_dir[BLOCK_SIZE / 64];
    path_info path_info1 = getpath(sourcepath);
    src_name = path_info1.file_name;
    src_block = path_info1.block;

    int dest_block;
    std::string dest_name;
    dir_entry dest_dir[BLOCK_SIZE / 64];
    path_info path_info2 = getpath(destpath);
    if (path_info2.block == -1 || path_info1.block == -1)
    {
        return -1;
    }
    dest_name = path_info2.file_name;
    dest_block = path_info2.block;

    disk.read(src_block, (uint8_t *)src_dir);
    disk.read(dest_block, (uint8_t *)dest_dir);
    if (dest_name.length() >= 56 || src_name.length() >= 56) // Check if the name is too long
    {
        std::cout << "Name too long\n";
        return -23;
    }
    if (dest_block == src_block && dest_name == src_name)
    {
        std::cout << "Need unique name\n";
        return -3;
    }
    // Looks for the folder and source file in root directory
    int dest_index = -1;
    int source_index = -1;
    for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
    {
        if (dest_index == -1 && dest_dir[i].file_name == dest_name)
        {
            // printf("FOUND FOLDER :D \n");
            dest_index = i;
        }
        if (source_index == -1 && src_dir[i].file_name == src_name && src_dir[i].type == 0) // Find the file which we're copying from
        {
            source_index = i;
        }
        if (strcmp(dest_dir[i].file_name, dest_name.c_str()) == 0 && dest_dir[i].type == 0) // Make sure no file has the same name
        {
            std::cout << "File already exists\n";
            return -7;
        }
    }
    if ((source_index) == -1) // incase no empty slot is found
    {
        std::cout << "file does not exist\n";
        return -3;
    }
    if (dest_index == -1)
    {
        char found = 0;
        // This happens if no folder is found with said name, then it is assumed that we want to create a new file with said name
        for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
        {
            if (found == 0 && dest_dir[i].type == 2)
            {
                found = 1;
                dest_index = i;
            }
            if (strcmp(dest_dir[i].file_name, dest_name.c_str()) == 0 && dest_dir[i].type == 0) // Make sure no file has the same name
            {
                std::cout << "File already exists\n";
                return -7;
            }
        }
    }
    if (dest_dir[dest_index].type == 1)
    {
        char found = 0;
        // HELP printf("Dest identified as dir\n");
        dest_block = dest_dir[dest_index].first_blk;
        disk.read(dest_block, (uint8_t *)dest_dir);
        for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
        {
            if (dest_dir[i].type == 2)
            {
                found = 1;

                dest_index = i;
            }
            if (strcmp(dest_dir[i].file_name, src_name.c_str()) == 0 && dest_dir[i].type == 0) // Make sure no file has the same name
            {
                std::cout << "File already exists\n";
                return -7;
            }
        }
        dest_name = src_name;
    }
    std::cout << dest_name << " THIS IS THE NAME OF our end file and it is written to " << dest_block << " FROM " << src_block << "\n";
    dest_dir[dest_index].access_rights = READ | WRITE;

    strcpy(dest_dir[dest_index].file_name, dest_name.c_str());
    std::cout << dest_dir[dest_index].file_name << "\n";
    dest_dir[dest_index].size = src_dir[source_index].size;
    dest_dir[dest_index].type = 0;
    int curr_block = -1;
    int read_block = src_dir[source_index].first_blk;
    std::cout << "reading words from " << read_block << "\n";
    while (read_block != FAT_EOF)
    {
        for (size_t i = 2; i < BLOCK_SIZE / 2; i++) // Will look for an empty place in the fat table
        {
            if (fat[i] == FAT_FREE) // Once an empty block is found, store index in curr_block, we will write to this block later
            {
                // printf("EMPTY BLOCK FOUND %d \n", i);
                if (curr_block != -1)
                    fat[curr_block] = i;
                else
                    dest_dir[dest_index].first_blk = i;
                curr_block = i;
                fat[curr_block] = FAT_EOF;
                char words[BLOCK_SIZE];
                // disk.read(read_block, (uint8_t *)words);

                // disk.write(curr_block, (uint8_t *)words);
                read_block = fat[read_block];
                printf("reading from next: %d\n ", read_block);
            }
            if (read_block == FAT_EOF)
                break;
        }
    }
    // updatesize(dest_dir[dest_index].size, dest_block);
    // dest_dir[dest_index].type = 0;
    // HELP std::cout << dest_block << "Slot is" << dest_index << " is the block we're writing to " << dest_dir[dest_index].file_name << " is the new file, the starting block is " << dest_dir[dest_index].first_blk << " type is" << dest_dir[dest_index].type << '\n';
    std::cout << dest_name.length() << " THIS IS THE NAME OF our end file and it is written to " << dest_block << " FROM " << src_block << "\n";
    std::cout << dest_dir[dest_index].file_name << "\n";
    std::cout << dest_dir[dest_index].size << " SIZE OF FILE " << dest_dir[dest_index].first_blk << " FIRST BLOCK \n";
    updatesize(dest_dir[dest_index].size, dest_block);
    disk.write(dest_block, (uint8_t *)dest_dir);
    // ls();
    std::cout << "FS::cp(" << sourcepath << "," << destpath << ")\n";
    return 0;
}
// FINISHED
//  mv <sourcepath> <destpath> renames the file <sourcepath> to the name <destpath>,
//  or moves the file <sourcepath> to the directory <destpath> (if dest is a directory)
int FS::mv(std::string sourcepath, std::string destpath)
{
    int src_block;
    std::string src_name;
    dir_entry src_dir[BLOCK_SIZE / 64];
    path_info path_info1 = getpath(sourcepath);
    src_name = path_info1.file_name;
    src_block = path_info1.block;

    int dest_block;
    std::string dest_name;
    dir_entry dest_dir[BLOCK_SIZE / 64];
    path_info path_info2 = getpath(destpath);
    if (path_info2.block == -1 || path_info1.block == -1)
    {
        return -1;
    }
    dest_name = path_info2.file_name;
    dest_block = path_info2.block;

    disk.read(src_block, (uint8_t *)src_dir);
    disk.read(dest_block, (uint8_t *)dest_dir);
    for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
    {
        if (dest_dir[i].file_name == dest_name && dest_dir[i].type == 1)
        {
            std::string haha = destpath + "/" + src_name;
            std::cout << "File copied to directory " << haha << "\n";

            cp(sourcepath, destpath); // if dest is a directory, then the last dir will be seen the name of the file according to our pathfinder
            rm(sourcepath);
            std::cout << "FS::mv(" << sourcepath << "," << destpath << ")\n";
            return 0;
        }
    }

    if (dest_name.length() >= 56 || src_name.length() >= 56) // Check if the name is too long
    {
        std::cout << "Name too long\n";
        return -2;
    }
    if (dest_block == src_block && dest_name == src_name)
    {
        std::cout << "Need unique name\n";
        return -3;
    }
    else if (dest_block == src_block)
    {
        // Looks for the file
        int sourceindex = -1;
        for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
        {
            if (sourceindex == -1 && dest_dir[i].file_name == sourcepath)
            {
                sourceindex = i;
            }
            if (strcmp(dest_dir[i].file_name, dest_name.c_str()) == 0) // Make sure no file has the same name
            {
                std::cout << "File already exists haha\n";
                return -2;
            }
        }
        if ((sourceindex) == -1) // incase no empty slot is found
        {
            std::cout << "File does not exist\n";
            return -3;
        }
        strcpy(dest_dir[sourceindex].file_name, dest_name.c_str());
        for (size_t i = 0; i < 64; i++)
        {
            // printf("%d is the type of %d\n", dest_dir[i].type, i);
        }
        disk.write(dest_block, (uint8_t *)dest_dir);
    }

    std::cout << "FS::mv(" << sourcepath << "," << destpath << ")\n";
    return 0;
}
// FINISHED
//  rm <filepath> removes / deletes the file <filepath>
int FS::rm(std::string filepath)
{
    int dest_block;
    std::string dest_name;
    dir_entry dest_dir[BLOCK_SIZE / 64];
    path_info path_info2 = getpath(filepath);
    if (path_info2.block == -1)
    {
        return -1;
    }
    dest_name = path_info2.file_name;
    dest_block = path_info2.block;

    disk.read(dest_block, (uint8_t *)dest_dir);

    if (dest_name.length() >= 56) // Check if the name is too long
    {
        std::cout << "Name too long\n";
        return -1;
    }
    // Copies over the file name to the new dir_entry's name

    // Looks for the file based on name
    int fileindex = -1;
    for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
    {
        if (fileindex == -1 && strcmp(dest_dir[i].file_name, dest_name.c_str()) == 0)
        {
            fileindex = i;
        }
    }
    // Gives error if one of the conditions are met
    if ((fileindex) == -1 || (dest_dir[fileindex].type == 1 && dest_dir[fileindex].size != 0))
    {
        std::cout << "File either doesn't exist or the directory is not empty\n";
        return -3;
    }
    dest_dir[fileindex].type = 2;
    updatesize(-dest_dir[fileindex].size, dest_block);
    dest_dir[fileindex].size = 0;
    strcpy(dest_dir[fileindex].file_name, "");
    // updatesize(-dest_dir[fileindex].size, dest_block);
    disk.write(dest_block, (uint8_t *)dest_dir);
    int fatindex = dest_dir[fileindex].first_blk;
    fat[fatindex] = FAT_FREE;

    while (fat[fatindex] != FAT_EOF)
    {
        int tmp = fat[fatindex];
        fat[fatindex] = FAT_FREE;
        fatindex = tmp;
    }

    std::cout << "FS::rm(" << filepath << ")\n";
    return 0;
}

// append <filepath1> <filepath2> appends the contents of file <filepath1> to
// the end of file <filepath2>. The file <filepath1> is unchanged.
int FS::append(std::string filepath1, std::string filepath2)
{
    if (filepath1 == filepath2) // Check if the name is too long
    {
        std::cout << "Appending to same file\n";
        return -1;
    }
    int src_block;
    std::string src_name;
    dir_entry src_dir[BLOCK_SIZE / 64];
    path_info path_info1 = getpath(filepath1);
    src_name = path_info1.file_name;
    src_block = path_info1.block;

    int dest_block;
    std::string dest_name;
    dir_entry dest_dir[BLOCK_SIZE / 64];
    path_info path_info2 = getpath(filepath2);
    if (path_info2.block == -1 || path_info1.block == -1)
    {
        return -1;
    }
    dest_name = path_info2.file_name;
    dest_block = path_info2.block;

    disk.read(src_block, (uint8_t *)src_dir);
    disk.read(dest_block, (uint8_t *)dest_dir);

    if (dest_name.length() >= 56 || src_name.length() >= 56) // Check if the name is too long
    {
        std::cout << "Name too long\n";
        return -2;
    }

    int destindex = -1;
    int sourceindex = -1;
    for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
    {
        if (sourceindex == -1 && src_dir[i].file_name == src_name)
        {
            sourceindex = i; // store index to later add our new dir_entry once all work is done
        }
        if (destindex == -1 && dest_dir[i].file_name == dest_name)
        {
            destindex = i;
        }
    }
    if (destindex == -1 || sourceindex == -1) // incase no empty slot is found
    {
        std::cout << "Either one file or both files do not exist\n";
        return -3;
    }
    if (dest_dir[destindex].type == 1 || src_dir[sourceindex].type == 1)
    {
        std::cout << " At least one of the files are a directory >:(\n";
        return -4;
    }
    if ((dest_dir[destindex].access_rights & WRITE) != WRITE)
    {
        return -5;
    }
    if ((src_dir[sourceindex].access_rights & READ) != READ)
    {
        return -6;
    }

    int lastblock = dest_dir[destindex].first_blk;
    while (fat[lastblock] != FAT_EOF)
    {
        lastblock = fat[lastblock];
        /* code */
    }
    int readblock = src_dir[sourceindex].first_blk;

    char contenttmp[4096];
    disk.read(lastblock, (uint8_t *)contenttmp);
    std::string content;

    content.append(contenttmp);
    content.append("\n");
    dest_dir[destindex].size -= content.length() - 1;
    while (readblock != FAT_EOF)
    {
        disk.read(readblock, (uint8_t *)contenttmp);
        content.append(contenttmp);
        readblock = fat[readblock];
    }
    while (content.length() >= BLOCK_SIZE)
    {
        std::string to_write = std::string(content.c_str(), BLOCK_SIZE);
        content.erase(BLOCK_SIZE);
        disk.write(lastblock, (uint8_t *)to_write.c_str());
        dest_dir[destindex].size += BLOCK_SIZE;
        char found = 0;
        for (size_t i = 0; i < BLOCK_SIZE / 2; i++) // look for a new block to write to
        {
            if (fat[i] == FAT_FREE)
            {
                found = 1;
                fat[lastblock] = i;
                lastblock = i;
                fat[lastblock] = FAT_EOF;
            }
        }
        if (found == 0)
        {
            std::cout << "Not enough space :( \n";
            return -7;
        }
    }
    if (content.length() > 0)
    {
        disk.write(lastblock, (uint8_t *)content.c_str());
        dest_dir[destindex].size += content.length();
    }
    // updatesize(src_dir[sourceindex].size, dest_block);
    disk.write(dest_block, (uint8_t *)dest_dir);
    std::cout
        << "FS::append(" << filepath1 << "," << filepath2 << ")\n";
    return 0;
}

// mkdir <dirpath> creates a new sub-directory with the name <dirpath>
// in the current directory
int FS::mkdir(std::string dirpath)
{
    int dest_block;
    dir_entry dest_dir[BLOCK_SIZE / 64];
    std::string filepath_arr = dirpath;
    path_info file_info = getpath(dirpath);
    dest_block = file_info.block;

    dir_entry parent; // Contains information about the parent directory
    strcpy(parent.file_name, "..");
    parent.type = 1;
    parent.first_blk = dest_block;
    disk.read(dest_block, (uint8_t *)dest_dir);
    dir_entry new_dir[BLOCK_SIZE / 64];

    int empty_slot = -1;
    for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
    {
        dir_entry empty_dir;
        empty_dir.type = 2;
        strcpy(empty_dir.file_name, "");
        new_dir[i] = empty_dir;                        // Initializes all the slots of our new directory
        if (empty_slot == -1 && dest_dir[i].type == 2) // Finds an empty slot in the working directory
        {
            empty_slot = i;
        }
        if (file_info.file_name == dest_dir[i].file_name)
        {
            printf("A file exists with said name \n");
            return -2;
        }
    }
    if (empty_slot == -1)
    {
        // No suitable location was found
        printf("No suitable location was found \n");
        return -3;
    }

    int child_block = -1;
    for (size_t i = 2; i < BLOCK_SIZE / 2; i++) // find a place in the fat table;
    {
        if (child_block == -1 && fat[i] == FAT_FREE)
        {
            child_block = i;
            fat[i] = FAT_EOF;
            break;
        }
    }
    if (child_block == -1)
    {
        return -4;
    }

    dir_entry child;

    child.first_blk = child_block;
    child.type = 1;
    strcpy(child.file_name, file_info.file_name.c_str());
    child.size = 0;

    new_dir[0] = parent;
    // printf("The created directory's entry is in %d and is being written to %d, which is stored in block %d \n", new_dir[0].first_blk, child_block, dest_block);
    dest_dir[empty_slot] = child;

    disk.write(child_block, (uint8_t *)new_dir);
    disk.write(dest_block, (uint8_t *)dest_dir);
    std::cout
        << "FS::mkdir(" << dirpath << ")\n";
    return 0;
}

// cd <dirpath> changes the current (working) directory to the directory named <dirpath>
int FS::cd(std::string dirpath)
{
    dir_entry dest_dir[BLOCK_SIZE / 64];
    path_info file_info = getpath(dirpath);
    disk.read(file_info.block, (uint8_t *)dest_dir);
    // std::cout << "Looking for " << dirpath << " in " << file_info.block << file_info.file_name << "\n";
    if (file_info.block_type != 1)
    {
        return -1;
    }

    char success = 0;
    for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
    {

        if (strcmp(dest_dir[i].file_name, file_info.file_name.c_str()) == 0 && dest_dir[i].type == 1)
        {
            success = 1;
            // printf("workingblock is now %d\n\n", dest_dir[i].first_blk);
            working_block = dest_dir[i].first_blk;
            break;
        }
    }
    if (success == 0)
    {
        printf("Could not find folder\n");
        return -2;
    }

    std::cout << "FS::cd(" << dirpath << ")\n";
    return 0;
}

// pwd prints the full path, i.e., from the root directory, to the current
// directory, including the currect directory name
int FS::pwd()
{
    // std::cout << "\nPWD\n";
    int search_dir = working_block;
    int next_dir = working_block; // will store the dir's block which we're trying to find
    dir_entry dir[BLOCK_SIZE / 64];
    std::string path = "";
    // printf("Going to block %d", dir[0].first_blk);
    disk.read(search_dir, (uint8_t *)dir);
    // HELP std::cout << search_dir << " PWD CORRECTLY read\n"
    //          << dir[0].type << "Is the type of the first entry\n";
    int root_searched = 0;
    while (1)
    {
        if (dir[0].type == 1 && strcmp(dir[0].file_name, "..") == 0)
        {
            next_dir = dir[0].first_blk;
            disk.read(dir[0].first_blk, (uint8_t *)dir);
        }
        else
            break;

        for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
        {
            if (search_dir == dir[i].first_blk && dir[i].type == 1)
            {
                // HELP std::cout << "We got here " << dir[i].file_name << "\n";

                path = "/" + (std::string)dir[i].file_name + path;
                // std::cout << path << "\n";
                search_dir = next_dir;

                // HELP std::cout << dir[0].first_blk << " is now the working block\n";
                break;
            }
        }
    }
    if (path == "")
    {
        path = '/';
    }
    std::cout << "FS::pwd()\n";
    std::cout << path << "\n";
    return 0;
}

// chmod <accessrights> <filepath> changes the access rights for the
// file <filepath> to <accessrights>.
int FS::chmod(std::string accessrights, std::string filepath)
{
    char right;
    if (accessrights.find('4') != std::string::npos)
    {
        right = right | READ;
    }
    if (accessrights.find('2') != std::string::npos)
    {
        right = right | WRITE;
    }
    if (accessrights.find('1') != std::string::npos)
    {
        right = right | EXECUTE;
    }

    path_info file = getpath(filepath);

    int index = -1;
    dir_entry dir[BLOCK_SIZE / 64];
    if (file.block == -1)
    {
        return -1;
    }

    disk.read(file.block, (uint8_t *)dir);

    for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
    {
        if (index == -1 && dir[i].type == 0 && strcmp(dir[i].file_name, file.file_name.c_str()) == 0)
        {
            index = i;
            dir[index].access_rights = right;
            disk.write(file.block, (uint8_t *)dir);
            break;
        }
    }
    if (index == -1)
    {
        return -1;
    }
    std::cout
        << "FS::chmod(" << accessrights << "," << filepath << ")\n";
    return 0;
}
