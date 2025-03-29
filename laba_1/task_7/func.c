#include "func.h"

status_code read_directory(DIR * dir, File ** files, size_t * count_files, const char * dir_path) {
    if (dir == NULL || files == NULL || count_files == NULL) {
        return PointerNullError;
    }

    *count_files = 0;
    size_t capacity_files = 2;
    *files = (File*)malloc(capacity_files * sizeof(File));
    if (*files == NULL) {
        return MemoryAllocationError;
    }

    struct stat file_stat;
    struct dirent * entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (*count_files == capacity_files) {
            capacity_files *= 2;
            File * tmp_files = (File*)realloc(*files, capacity_files * sizeof(File));
            if (tmp_files == NULL) {
                for (size_t i = 0; i < *count_files; i++) {
                    free((*files)[i].owner);
                    free((*files)[i].group);
                    free((*files)[i].name);
                }
                free(*files);
                *files = NULL;
                return MemoryAllocationError;
            }
            *files = tmp_files;
        }

        // Формируем полный путь к файлу
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        if (stat(full_path, &file_stat) == -1) {
            for (size_t i = 0; i < *count_files; i++) {
                free((*files)[i].owner);
                free((*files)[i].group);
                free((*files)[i].name);
            }
            free(*files);
            *files = NULL;
            return StatError;
        }

        // Тип файла
        if (S_ISDIR(file_stat.st_mode)) {
            (*files)[*count_files].type_and_rights[0] = 'd';
        } else if (S_ISREG(file_stat.st_mode)) {
            (*files)[*count_files].type_and_rights[0] = '-';
        } else if (S_ISLNK(file_stat.st_mode)) {
            (*files)[*count_files].type_and_rights[0] = 'l';
        } else if (S_ISFIFO(file_stat.st_mode)) {
            (*files)[*count_files].type_and_rights[0] = 'p';
        } else if (S_ISCHR(file_stat.st_mode)) {
            (*files)[*count_files].type_and_rights[0] = 'c';
        } else if (S_ISBLK(file_stat.st_mode)) {
            (*files)[*count_files].type_and_rights[0] = 'b';
        }

        // Права доступа
        (*files)[*count_files].type_and_rights[1] = (file_stat.st_mode & S_IRUSR) ? 'r' : '-'; 
        (*files)[*count_files].type_and_rights[2] = (file_stat.st_mode & S_IWUSR) ? 'w' : '-'; 
        (*files)[*count_files].type_and_rights[3] = (file_stat.st_mode & S_IXUSR) ? 'x' : '-';
        (*files)[*count_files].type_and_rights[4] = (file_stat.st_mode & S_IRGRP) ? 'r' : '-'; 
        (*files)[*count_files].type_and_rights[5] = (file_stat.st_mode & S_IWGRP) ? 'w' : '-'; 
        (*files)[*count_files].type_and_rights[6] = (file_stat.st_mode & S_IXGRP) ? 'x' : '-';
        (*files)[*count_files].type_and_rights[7] = (file_stat.st_mode & S_IROTH) ? 'r' : '-'; 
        (*files)[*count_files].type_and_rights[8] = (file_stat.st_mode & S_IWOTH) ? 'w' : '-'; 
        (*files)[*count_files].type_and_rights[9] = (file_stat.st_mode & S_IXOTH) ? 'x' : '-'; 
        (*files)[*count_files].type_and_rights[10] = '\0';

        // Количество жестких ссылок
        (*files)[*count_files].count_hard_links = file_stat.st_nlink;

        // Имя владельца
        uid_t owner_uid = file_stat.st_uid;
        struct passwd *owner_info = getpwuid(owner_uid);
        if (owner_info != NULL) {
            (*files)[*count_files].owner = strdup(owner_info->pw_name);
        } else {
            (*files)[*count_files].owner = strdup(""); 
        }
        if ((*files)[*count_files].owner == NULL) {
            for (size_t i = 0; i < *count_files; i++) {
                free((*files)[i].owner);
                free((*files)[i].group);
                free((*files)[i].name);
            }
            free(*files);
            *files = NULL;
            return MemoryAllocationError;
        }

        // Название группы
        gid_t group_gid = file_stat.st_gid;
        struct group *group_info = getgrgid(group_gid);
        if (group_info != NULL) {
            (*files)[*count_files].group = strdup(group_info->gr_name);
        } else {
            (*files)[*count_files].group = strdup(""); 
        }
        if ((*files)[*count_files].group == NULL) {
            for (size_t i = 0; i < *count_files; i++) {
                free((*files)[i].owner);
                free((*files)[i].group);
                free((*files)[i].name);
            }
            free(*files);
            *files = NULL;
            return MemoryAllocationError;
        }

        // Размер файла
        (*files)[*count_files].size = file_stat.st_size;

        // Дата изменения
        struct tm * time_info = localtime(&file_stat.st_mtime);
        char update_time[14];
        strftime(update_time, sizeof(update_time), "%b %e %H:%M", time_info);
        strncpy((*files)[*count_files].update_time, update_time, sizeof((*files)[*count_files].update_time) - 1);
        (*files)[*count_files].update_time[sizeof((*files)[*count_files].update_time) - 1] = '\0'; 

        // Имя файла
        (*files)[*count_files].name = strdup(entry->d_name);
        if ((*files)[*count_files].name == NULL) {
            for (size_t i = 0; i < *count_files; i++) {
                free((*files)[i].owner);
                free((*files)[i].group);
                free((*files)[i].name);
            }
            free(*files);
            *files = NULL;
            return MemoryAllocationError;
        }

        // Дисковый адрес
        (*files)[*count_files].inode = file_stat.st_ino;

        (*count_files)++;
    }

    return OK;
}


void print_info(File * files, size_t count_files){
    if (files == NULL){return;}
    for (size_t i = 0; i < count_files; i++){
        printf("%s %ld %s %s %ld %s %s %ld\n", files[i].type_and_rights, files[i].count_hard_links, files[i].owner, files[i].group, files[i].size, files[i].update_time, files[i].name, files[i].inode);
    }
}

void free_array(File ** files, size_t count_files){
    if (files == NULL){return;}
    for (size_t i = 0; i < count_files; i++){
        free((*files)[i].owner);
        free((*files)[i].group);
        free((*files)[i].name); 
    }
    free(*files); *files = NULL;
}
