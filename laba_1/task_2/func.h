#ifndef FUNC_H
#define FUNC_H

#include <ctype.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdarg.h>


typedef enum {
    OK,
    PonterNullError,
    InvalidInput,
    MemoryAllocationError,
    OpenFileError,
    ErrorPid,
    NumberError,
}status_code;


status_code CheckN(const char *N);
status_code CopyN(const char *FileName, const int *NeedCopies);
status_code FindString(const char **Files, int FileCount, const char *pattern, char *FoundIn, char *FlagFound);
status_code String_To_uint32_t (const char *str, u_int32_t *result);
status_code Xor(const char *FileName, int N, u_int64_t *result);
status_code Mask(const char *FileName, const char *mask, int *count);

#endif