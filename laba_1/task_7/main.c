#include "func.h"

int main(int argc, char *argv[]){
    if (argc == 1){
        DIR *dir = opendir(".");
        if (dir == NULL){
            printf("Не удалось открыть данный каталог\n");
            return OpenDirError;
        }
    
        File * files = NULL;
        size_t count_files= 0;
        status_code status = read_directory(dir, &files, &count_files, ".");
        if (status != OK){
            if (status == MemoryAllocationError){
                printf("Не удалось выделить память\n");
            }
            if (status == PointerNullError){
                printf("В функцию были переданы параметры со значением NULL\n");
            }
            if(status == StatError){
                printf("Stat error\n");
            }
            free_array(&files, count_files);
            closedir(dir);
            return status;
        }
    
        print_info(files, count_files);
        free_array(&files, count_files);
        closedir(dir);
    }
    else {
        DIR *dir;
        File * files = NULL;
        size_t count_files = 0;
        status_code status;
        
        for (size_t count_dir = 1; count_dir < argc; count_dir++){
            printf("Для каталога %s: \n", argv[count_dir]);
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "./%s", argv[count_dir]);
            dir = opendir(full_path);
            if (dir == NULL){
                printf("Каталог '%s' не найден в текущей директории\n", argv[count_dir]);
                return OpenDirError;
            }

            status = read_directory(dir, &files, &count_files, argv[count_dir]);
            if (status != OK){
                if (status == MemoryAllocationError){
                    printf("Не удалось выделить память\n");
                }
                if (status == PointerNullError){
                    printf("В функцию были переданы параметры со значением NULL\n");
                }
                if(status == StatError){
                    printf("Не удалось получить данные по файлу\n");
                }
                free_array(&files, count_files);
                closedir(dir);
                return status;
            }
            print_info(files, count_files);
            free_array(&files, count_files);
            count_files = 0;  
            closedir(dir);
            if (count_dir + 1 != argc){
                printf("\n");
            }
        }
    }

    return 0;
}
