#ifndef FUNC_H
#define FUNC_H

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h> 
#include <sys/types.h>
#include <grp.h> 
#include <time.h>
#include <unistd.h>

typedef enum{
    OK,
    PointerNullError,
    OpenDirError,
    MemoryAllocationError,
    InvalidInput,
    StatError
}status_code;

typedef struct {
    char type_and_rights[11]; 
    size_t count_hard_links;   
    char *owner;              
    char *group;               
    off_t size;               
    char update_time[14];      
    char *name;                
    ino_t inode;              
}File;


status_code read_directory(DIR * dir, File ** files, size_t * count_files, const char * dir_path);
void print_info(File * files, size_t  count_files);
void free_array(File ** files, size_t count_files);


#endif
