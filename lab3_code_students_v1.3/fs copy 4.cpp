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
    if (filepath[0] == '/') // Sets the directory we'll look from to the root
    {
        disk.read(ROOT_BLOCK, (u_int8_t *)dest_dir);
        subdir_block = ROOT_BLOCK;
        filepath_arr.erase(0, 1);
    }
    else // starts the search in the block in which we're currently in
    {
        subdir_block = working_block;
        disk.read(working_block, (u_int8_t *)dest_dir);
    }

    while (1)
    {
        std::string subdir_name;
        size_t subdir_index = filepath_arr.find('/');
        if (subdir_index == std::string::npos) // npos is returned when find doesn't find what its looking for.
        {                                      // we have thus reached our destination dir.
            file_name = filepath_arr;          // the remaining characters are the name of file/dir which we want to work with
            dest_block = subdir_block;
            break; // exits the while loop to prepare data to return it.
        }
        subdir_name = filepath_arr.substr(0, subdir_index); // gets name of the subdirectory we're supposed to look for
        filepath_arr.erase(0, subdir_index + 1);            // removes the subdir from the entire the total string
        char found = 0;                                     // used for error handling

        for (size_t i = 0; i < BLOCK_SIZE / 64; i++) // looks for current subdirectory in parent dir
        {
            if (strcmp(subdir_name.c_str(), dest_dir[i].file_name) == 0 && dest_dir[i].type == 1)
            {
                found = 1;
                subdir_block = dest_dir[i].first_blk;
                index = i;
                disk.read(subdir_block, (uint8_t *)dest_dir); // reads in the subdir to repeat the process
                break;
            }
        }
        if (found == 0)
        {
            return path; // this simply exists in case a faulty path is given. as an error
        }
    }
    path.block = dest_block;
    path.file_name = file_name;
    if (dest_block != 0)
    {
        path.block_type = dest_dir[index].type;
    }
    else
        path.block_type = 1;
    return path;
}
void FS::updatesize(uint32_t size, int originblock) // iteratively works back and updates the size of each parent dir
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
    fat[FAT_BLOCK] = FAT_EOF; // root and fat block are always supposed to be occupied so they're set seperately

    for (size_t i = 0; i < BLOCK_SIZE / 64; i++) // a non assigned type value is set to denote an empty slot in the dir
    {
        root[i].type = 2;
    }

    disk.write(ROOT_BLOCK, (uint8_t *)root); // writes both fat and root to the disk
    disk.write(FAT_BLOCK, (uint8_t *)fat);
    std::cout
        << "FS::format()\n";
    return 0;
}
//  create <filepath> creates a new file on the disk, the data content is
//  written on the following rows (ended with an empty row)
int FS::create(std::string filepath)
{
    path_info path_info = getpath(filepath);
    if (path_info.block == -1) // incase the directory isn't found for one reason or another
    {
        return -1;
    }

    dir_entry dest_dir[BLOCK_SIZE / 64];
    std::string file_name = path_info.file_name;
    int dest_block = path_info.block;

    if (file_name.length() >= 56) // Check if the name is too long
    {
        return -1;
    }
    disk.read(dest_block, (uint8_t *)dest_dir);
    // Copies over the file name to the new dir_entry's name

    int open = -1;
    for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
    {

        if (open == -1 && dest_dir[i].type == 2) // Each empty slot in root directory has a dir_entry with the type value of 2
        {
            open = i; // store index to later add our new dir_entry once all work is done
            break;
        }
        if (strcmp(dest_dir[i].file_name, file_name.c_str()) == 0) // Make sure a duplicate isn't created
        {
            return -2;
        }
    }
    if (open == -1) // If open equals -1 then no empty slot exists in the root
    {
        printf("Did not find a slot \n");
        return -3;
    }
    std::string input;   // will store user input
    int curr_block = -1; // Will store the current block which we're writing the characters to

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
        getline(std::cin, in);
        dest_dir[open].size++;
        if (in.length() <= 1 || in == "\n") // once the temporary variable receives an empty new line, break the loop
        {
            input.append(in);
            disk.write(curr_block, (uint8_t *)input.c_str()); // write remaining input to the block
            fat[curr_block] = FAT_EOF;
            dest_dir[open].size += input.length() - 1;
            break;
        }

        input.append(in); // appends this iteration's input to the total input for our block

        while (input.length() >= BLOCK_SIZE)                               // need to find a new block to write to once we have more than 4096 bytes of info
        {                                                                  // in the input array
            std::string to_write = std::string(input.c_str(), BLOCK_SIZE); // stores the stuff we will write to the block
            disk.write(curr_block, (uint8_t *)to_write.c_str());           // writes to current block
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
    updatesize(dest_dir[open].size, dest_block);
    disk.write(dest_block, (uint8_t *)dest_dir);
    disk.write(FAT_BLOCK, (uint8_t *)fat);
    std::cout
        << "FS::create(" << filepath << ")\n";
    return 0;
}
// cat <filepath> reads the content of a file and prints it on the screen
int FS::cat(std::string filepath)
{
    path_info path_info = getpath(filepath);
    if (path_info.block == -1)
    {
        return -1;
    }

    dir_entry dest_dir[BLOCK_SIZE / 64];
    std::string file_name = path_info.file_name;
    int dest_block = path_info.block;
    disk.read(dest_block, (uint8_t *)dest_dir);
    int pos = -1;

    for (size_t i = 0; i < BLOCK_SIZE / 64; i++) // looks for the file
    {
        if (strcmp(dest_dir[i].file_name, file_name.c_str()) == 0)
        {
            pos = dest_dir[i].first_blk;
            break;
        }
    }
    if (pos == -1 || dest_dir[pos].type == 1) // if no file is found, or the file is a directory
    {
        return -2;
    }
    if ((dest_dir[pos].access_rights & READ) != READ) // using bitwise and to check rights of the file :)
    {
        return -3;
    }
    // looks until
    std::string out;
    char tmp[4096];
    { // reads in each block one at a time and stores it in out variable.
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
    disk.read(working_block, (uint8_t *)root); // read in working directory
    std::cout << "FS::ls()\n";
    std::cout << "name\t type\t accessrights\t size \n"; // wished format (the order of the attributes may not be correct)
    for (size_t i = 0; i < BLOCK_SIZE / 64; i++)         // iterates through each element in the list and prints the properties of the entries
    {
        if ((root[i].type == 1) || (root[i].type == 0) && (root[i].first_blk != -1))
        {
            std::string filetype;
            std::string rights = "---";
            if (root[i].type == 1)
                filetype = "dir";
            if (root[i].type == 0)
                filetype = "file";
            if ((root[i].access_rights & READ) == READ) // the following statments format the rwx rights in the requested way (i think)
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
                rights[2] = 'x';
                if (rights[0] != 'r')
                {
                    rights[0] = ' ';
                }
            }

            std::cout << root[i].file_name << "\t" << filetype << "\t" << rights << "\t" << root[i].size << "\t\n";
        }
    }

    return 0;
}

//  cp <sourcepath> <destpath> makes an exact copy of the file
//  <sourcepath> to a new file <destpath>
int FS::cp(std::string sourcepath, std::string destpath)
{
    path_info path_info1 = getpath(sourcepath);           // calling get path with the different paths
    path_info path_info2 = getpath(destpath);             // yields all the information required
    if (path_info2.block == -1 || path_info1.block == -1) // in case something went wrong in either of the paths
    {
        return -1;
    }
    dir_entry src_dir[BLOCK_SIZE / 64];
    dir_entry dest_dir[BLOCK_SIZE / 64];
    std::string src_name = path_info1.file_name; // stores the information for ease of use
    std::string dest_name = path_info2.file_name;
    int src_block = path_info1.block;
    int dest_block = path_info2.block;

    disk.read(src_block, (uint8_t *)src_dir); // read in both directories
    disk.read(dest_block, (uint8_t *)dest_dir);
    if (dest_name.length() >= 56 || src_name.length() >= 56) // Check if either name is too long
    {
        return -2;
    }
    if (dest_block == src_block && dest_name == src_name) // if src_block & dest_block are the same then the names have to be different
    {
        return -3;
    }
    // Looks for the folder and source file in root directory
    int dest_index = -1;
    int source_index = -1;
    for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
    {
        if (dest_index == -1 && dest_dir[i].type == 1 && dest_dir[i].file_name == dest_name) // If the file name is a directory then we need to read in that dir
        {
            dest_index = i;
        }
        if (source_index == -1 && src_dir[i].file_name == src_name && src_dir[i].type == 0) // Find the file which we're copying from
        {
            source_index = i;
        }
        if (strcmp(dest_dir[i].file_name, dest_name.c_str()) == 0 && dest_dir[i].type == 0) // Make sure no file has the same name
        {
            return -7;
        }
    }
    if ((source_index) == -1) // incase no empty slot is found
    {
        return -3;
    }
    if (dest_dir[dest_index].type == 1) // goes into the folder in which a copy is supposed to be made
    {
        dest_block = dest_dir[dest_index].first_blk;
        disk.read(dest_block, (uint8_t *)dest_dir);
        dest_name = src_name;
    }
    char found = 0;
    for (size_t i = 0; i < BLOCK_SIZE / 64; i++) // looks for an empty slot in the dest directory
    {
        if (found == 0 && dest_dir[i].type == 2)
        {
            found = 1;
            dest_index = i;
        }
        if (strcmp(dest_dir[i].file_name, dest_name.c_str()) == 0 && dest_dir[i].type == 0) // Make sure no file has the same name
        {
            return -7;
        }
    }

    // updates the direntry with the src information
    dest_dir[dest_index].access_rights = src_dir[source_index].access_rights;
    strcpy(dest_dir[dest_index].file_name, dest_name.c_str());
    dest_dir[dest_index].size = src_dir[source_index].size;
    dest_dir[dest_index].type = 0;
    // copies over src contents to new blocks to make them seperate files
    int curr_block = -1;
    int read_block = src_dir[source_index].first_blk;
    while (read_block != FAT_EOF)
    {
        for (size_t i = 2; i < BLOCK_SIZE / 2; i++) // Will look for an empty place in the fat table
        {
            if (fat[i] == FAT_FREE) // Once an empty block is found, store index in curr_block, we will write to this block later
            {
                if (curr_block != -1)
                    fat[curr_block] = i;
                else
                    dest_dir[dest_index].first_blk = i;
                curr_block = i;
                fat[curr_block] = FAT_EOF;
                char words[BLOCK_SIZE];
                read_block = fat[read_block];
            }
            if (read_block == FAT_EOF)
                break;
        }
    }
    updatesize(dest_dir[dest_index].size, dest_block);
    disk.write(dest_block, (uint8_t *)dest_dir);
    std::cout << "FS::cp(" << sourcepath << "," << destpath << ")\n";
    return 0;
}
//  mv <sourcepath> <destpath> renames the file <sourcepath> to the name <destpath>,
//  or moves the file <sourcepath> to the directory <destpath> (if dest is a directory)
int FS::mv(std::string sourcepath, std::string destpath)
{
    path_info path_info1 = getpath(sourcepath);
    printf("igothereee\n");
    path_info path_info2 = getpath(destpath);
    printf("igothereee\n");
    if (path_info2.block == -1 || path_info1.block == -1) // if something went wrong in finding paths
    {
        return -1;
    }
    std::string src_name = path_info1.file_name; // for ease of use
    std::string dest_name = path_info2.file_name;
    int src_block = path_info1.block;
    int dest_block = path_info2.block;

    dir_entry src_dir[BLOCK_SIZE / 64];
    dir_entry dest_dir[BLOCK_SIZE / 64];

    if (dest_name.length() >= 56 || src_name.length() >= 56) // Check if the name is too long
    {
        return -2;
    }
    if (dest_block == src_block && dest_name == src_name)
    {
        return -3;
    }
    disk.read(src_block, (uint8_t *)src_dir);
    disk.read(dest_block, (uint8_t *)dest_dir);
    char isdir = 0;
    // Looks for the file
    int src_index = -1;
    for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
    {
        if (src_index == -1 && strcmp(src_dir[i].file_name, src_name.c_str()) == 0)
        {
            src_index = i;
        }
    }
    if ((src_index) == -1) // incase the file isn't found
    {
        return -3;
    }
    for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
    {
        if (dest_dir[i].file_name == dest_name && dest_dir[i].type == 1) // if a destname is the name of dir then we want to move the dir entry
        {
            printf("igot here\n");
            disk.read(dest_dir[i].first_blk, (uint8_t *)dest_dir);
            dest_name = src_name;
            int found = 0;
            int dest_index = -1;
            for (size_t j = 0; j < BLOCK_SIZE / 64; j++)
            {
                isdir = 1;
                if (found == 0 && dest_dir[j].type == 2)
                {
                    found = 1;
                    dest_index = j;
                }
                if (strcmp(dest_dir[j].file_name, dest_name.c_str()) == 0) // make sure no file has the same name
                {
                    return -2;
                }
            }
            if (found == 0) // if no empty slot is found
            {
                return -3;
            }
            printf("hallao\n");
            dest_dir[dest_index] = src_dir[src_index];
            src_dir[src_index].type = 0;
            strcpy(src_dir[src_index].file_name, " ");
            updatesize(src_dir[src_index].size, i);
            updatesize(-src_dir[src_index].size, src_block);
            disk.write(dest_block, (uint8_t *)dest_dir);
            disk.write(src_block, (uint8_t *)src_dir);
            printf("fibished\n");
            std::cout << "FS::mv(" << sourcepath << "," << destpath << ")\n";
            return 0;
        }
    }
    for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
    {
        if (strcmp(dest_dir[i].file_name, dest_name.c_str()) == 0) // Make sure no file has the same name
        {
            return -2;
        }
    }

    strcpy(dest_dir[src_index].file_name, dest_name.c_str());
    disk.write(dest_block, (uint8_t *)dest_dir);

    std::cout << "FS::mv(" << sourcepath << "," << destpath << ")\n";
    return 0;
}
// FINISHED
//  rm <filepath> removes / deletes the file <filepath>
int FS::rm(std::string filepath)
{
    path_info path_info2 = getpath(filepath);
    if (path_info2.block == -1)
    {
        return -1;
    }
    std::string dest_name = path_info2.file_name;
    int dest_block = path_info2.block;
    dir_entry dest_dir[BLOCK_SIZE / 64];
    disk.read(dest_block, (uint8_t *)dest_dir);

    if (dest_name.length() >= 56) // Check if the name is too long
    {
        return -1;
    }
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
        return -3;
    }
    // nulls the entry
    dest_dir[fileindex].type = 2;
    updatesize(-dest_dir[fileindex].size, dest_block);
    dest_dir[fileindex].size = 0;
    strcpy(dest_dir[fileindex].file_name, "");
    disk.write(dest_block, (uint8_t *)dest_dir);
    int fatindex = dest_dir[fileindex].first_blk;
    fat[fatindex] = FAT_FREE;

    while (fat[fatindex] != FAT_EOF) // marks all the files fat entries as free
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
    if (filepath1 == filepath2) // can't append to the same file
    {
        return -1;
    }
    path_info path_info1 = getpath(filepath1);
    path_info path_info2 = getpath(filepath2);
    if (path_info2.block == -1 || path_info1.block == -1) // errors from getting path
    {
        return -1;
    }

    std::string src_name = path_info1.file_name;
    int src_block = path_info1.block;
    std::string dest_name = path_info2.file_name;
    int dest_block = path_info2.block;

    dir_entry src_dir[BLOCK_SIZE / 64];
    dir_entry dest_dir[BLOCK_SIZE / 64];
    disk.read(src_block, (uint8_t *)src_dir);
    disk.read(dest_block, (uint8_t *)dest_dir);

    if (dest_name.length() >= 56 || src_name.length() >= 56) // Check if the name is too long
    {
        return -2;
    }

    int destindex = -1;
    int sourceindex = -1;
    for (size_t i = 0; i < BLOCK_SIZE / 64; i++) // look for both files
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
        return -3;
    }
    if (dest_dir[destindex].type == 1 || src_dir[sourceindex].type == 1) // if either file is a dir
    {
        return -4;
    }
    if ((dest_dir[destindex].access_rights & WRITE) != WRITE) // access rights for each file we're using
    {
        return -5;
    }
    if ((src_dir[sourceindex].access_rights & READ) != READ)
    {
        return -6;
    }

    int lastblock = dest_dir[destindex].first_blk; // looks for the last block in our destination file
    while (fat[lastblock] != FAT_EOF)
    {
        lastblock = fat[lastblock];
    }
    int readblock = src_dir[sourceindex].first_blk; // will append from the first block in srcfile

    char contenttmp[4096];
    disk.read(lastblock, (uint8_t *)contenttmp);
    std::string content;

    content.append(contenttmp);                       // adds the last block's content to be written later back in so we don't have to look for a new block immediately.
    content.append("\n");                             // adds the new line which was supposed to be added in create.
    dest_dir[destindex].size -= content.length() - 1; // removes from the file size temporarily
    while (readblock != FAT_EOF)                      // reads all the contents
    {
        disk.read(readblock, (uint8_t *)contenttmp);
        content.append(contenttmp);
        readblock = fat[readblock];
    }
    while (content.length() >= BLOCK_SIZE) // constantly write to a block look for a block
    {                                      // until we have less than a blocks worth of content left
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
        if (found == 0) // if fat is full return errors
        {
            return -7;
        }
    }
    if (content.length() > 0) // writes all leftovers to the current block
    {
        disk.write(lastblock, (uint8_t *)content.c_str());
        dest_dir[destindex].size += content.length();
    }
    disk.write(dest_block, (uint8_t *)dest_dir); // update the directory which contains the modified file
    std::cout
        << "FS::append(" << filepath1 << "," << filepath2 << ")\n";
    return 0;
}
// mkdir <dirpath> creates a new sub-directory with the name <dirpath>
// in the current directory
int FS::mkdir(std::string dirpath)
{
    dir_entry dest_dir[BLOCK_SIZE / 64];
    path_info file_info = getpath(dirpath);
    int dest_block = file_info.block;

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
        if (file_info.file_name == dest_dir[i].file_name) // if an existing file or dir has the name
        {
            return -2;
        }
    }
    if (empty_slot == -1) // No suitable location was found
    {
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
            working_block = dest_dir[i].first_blk;
            break;
        }
    }
    if (success == 0) // if a folder wasn't found for some reason (something horrible must've happened for this to happen)
    {
        return -2;
    }

    std::cout << "FS::cd(" << dirpath << ")\n";
    return 0;
}

// pwd prints the full path, i.e., from the root directory, to the current
// directory, including the currect directory name
int FS::pwd()
{
    int search_dir = working_block;
    int next_dir = working_block; // will store the dir's block which we're trying to find
    dir_entry dir[BLOCK_SIZE / 64];
    std::string path = "";
    disk.read(search_dir, (uint8_t *)dir);
    int root_searched = 0;
    while (1) // works backward from each directory
    {
        if (dir[0].type == 1 && strcmp(dir[0].file_name, "..") == 0) // uses first entry to go to parent to look for the subdir
        {                                                            // if dir[0] is not of a dir type, then we've either
            next_dir = dir[0].first_blk;                             // done something really wrong or we've reached root
            disk.read(dir[0].first_blk, (uint8_t *)dir);
        }
        else
            break;

        for (size_t i = 0; i < BLOCK_SIZE / 64; i++)
        {
            if (search_dir == dir[i].first_blk && dir[i].type == 1) // add the curernt subdir to the path which will be printed
            {
                path = "/" + (std::string)dir[i].file_name + path;
                search_dir = next_dir;
                break;
            }
        }
    }
    if (path == "") // root '/' :)
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
    if (accessrights.find('4') != std::string::npos) // I assume you can send in multiple rights at one time
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
        if (index == -1 && dir[i].type == 0 && strcmp(dir[i].file_name, file.file_name.c_str()) == 0) // updates the file
        {
            index = i;
            dir[index].access_rights = right;
            disk.write(file.block, (uint8_t *)dir);
            break;
        }
    }
    if (index == -1) // the file was not found
    {
        return -1;
    }
    std::cout << "FS::chmod(" << accessrights << "," << filepath << ")\n";
    return 0;
}
