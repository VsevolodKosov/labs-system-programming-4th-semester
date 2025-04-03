#include "func.h"

status_code CopyN(const char *FileName, const int *NeedCopies) {
    if (!FileName || !NeedCopies || *NeedCopies <= 0) {
        return PonterNullError;
    }

    FILE *test_file = fopen(FileName, "r");
    if (!test_file) {
        return OpenFileError;
    }
    fclose(test_file);

    for (int i = 0; i < *NeedCopies; ++i) {
        pid_t pid = fork();

        if (pid < 0) {
            return ErrorPid;
        }

        if (pid == 0) {
            size_t buf_size = 256;
            char *NewFileName = (char *)malloc(buf_size);
            if (!NewFileName) {
                return MemoryAllocationError;
            }

            int needed;
            while ((needed = snprintf(NULL, 0, "%s_%d.txt", FileName, i + 1)) >= buf_size) {
                buf_size = needed + 1;
                char *new_buf = (char *)realloc(NewFileName, buf_size);
                if (!new_buf) {
                    free(NewFileName);
                    return MemoryAllocationError;
                }
                NewFileName = new_buf;
            }
            snprintf(NewFileName, buf_size, "%s_%d.txt", FileName, i + 1);

            FILE *src = fopen(FileName, "r");
            FILE *dst = fopen(NewFileName, "w");

            if (!src || !dst) {
                if (src) fclose(src);
                if (dst) fclose(dst);
                free(NewFileName);
                return OpenFileError;
            }

            int ch;
            while ((ch = fgetc(src)) != EOF) {
                fputc(ch, dst);
            }

            fclose(src);
            fclose(dst);
            free(NewFileName);
            exit(OK);
        }
    }

    while (wait(NULL) > 0);

    return OK;
}

status_code Xor(const char *FileName, int N, u_int64_t *result) {
    if (!FileName || !result) {
        return PonterNullError;
    }

    uint64_t BlockSizeBits = 1ULL << N;
    uint64_t XorSum = 0, CurrentBlockValue = 0;
    uint64_t BitCount = 0;
    int i, CurrentBit;
    uint8_t byte;
    FILE *file;

    file = fopen(FileName, "rb");
    if (!file) {
        return OpenFileError;
    }

    while (fread(&byte, 1, 1, file) == 1) {
        for (i = 0; i < 8; ++i) {
            CurrentBit = (byte >> i) & 1;
            CurrentBlockValue |= (CurrentBit << BitCount);
            BitCount++;

            if (BitCount == BlockSizeBits) {
                XorSum ^= CurrentBlockValue;
                CurrentBlockValue = 0;
                BitCount = 0;
            }
        }
    }

    if (BitCount > 0) {
        CurrentBlockValue <<= (BlockSizeBits - BitCount);
        XorSum ^= CurrentBlockValue;
    }

    fclose(file);
    *result = XorSum;
    return OK;
}

status_code Mask(const char *FileName, const char *mask, int *count) {
    if (!FileName || !mask || !count) {
        return MemoryAllocationError;
    }

    uint32_t WriteMask, num32;
    int NewCount = 0;
    size_t tst;
    FILE *file;

    file = fopen(FileName, "rb");
    if (!file) {
        return OpenFileError;
    }

    if (strlen(mask) > 8 || String_To_uint32_t(mask, &WriteMask)) {
        fclose(file);
        return InvalidInput;
    }

    while ((tst = fread(&num32, sizeof(uint32_t), 1, file)) == 1) {
        if (num32 == WriteMask) {
            NewCount++;
        }
    }

    fclose(file);
    *count = NewCount;
    return OK;
}

status_code FindString(const char **Files, int FileCount, const char *pattern, char *FoundIn, char *FlagFound) {
    if (!Files || !FoundIn || !FlagFound) {
        return PonterNullError;
    }

    char *processed_pattern = NULL;
    int pattern_size = 0;
    int escape = 0;
    int final_length = 0;

    for (int i = 0; pattern[i]; i++) {
        if (escape) {
            final_length++;
            escape = 0;
        } else if (pattern[i] == '\\') {
            escape = 1;
        } else {
            final_length++;
        }
    }

    processed_pattern = (char *)malloc(final_length + 1);
    if (!processed_pattern) {
        return MemoryAllocationError;
    }

    escape = 0;
    for (int i = 0; pattern[i]; i++) {
        if (escape) {
            switch (pattern[i]) {
                case 'n':  processed_pattern[pattern_size++] = '\n'; break;
                case 't':  processed_pattern[pattern_size++] = '\t'; break;
                case 'r':  processed_pattern[pattern_size++] = '\r'; break;
                case '0':  processed_pattern[pattern_size++] = '\0'; break;
                case '\\': processed_pattern[pattern_size++] = '\\'; break;
                default:   processed_pattern[pattern_size++] = pattern[i]; break;
            }
            escape = 0;
        } else if (pattern[i] == '\\') {
            escape = 1;
        } else {
            processed_pattern[pattern_size++] = pattern[i];
        }
    }
    processed_pattern[pattern_size] = '\0';

    int shm_id = shmget(IPC_PRIVATE, (FileCount + 1) * sizeof(char), IPC_CREAT | 0666);
    if (shm_id == -1) return MemoryAllocationError;

    char *shared = (char *)shmat(shm_id, NULL, 0);
    if (shared == (void *)-1) {
        shmctl(shm_id, IPC_RMID, NULL);
        return MemoryAllocationError;
    }
    memset(shared, 0, FileCount + 1);

    for (int i = 0; i < FileCount; ++i) {
        pid_t pid = fork();
        if (pid < 0) {
            shmdt(shared);
            shmctl(shm_id, IPC_RMID, NULL);
            return ErrorPid;
        }

        if (pid == 0) {
            FILE *file = fopen(Files[i], "rb");
            if (!file) return OpenFileError;

            int found = 0;
            int current_pos = 0;
            int ch;
            int ignore_cr = 0;

            while ((ch = fgetc(file)) != EOF && !found) {
                if (ch == '\r') {
                    ignore_cr = 1;
                    continue;
                }

                if (ch == (unsigned char)processed_pattern[current_pos]) {
                    if (++current_pos == pattern_size) {
                        found = 1;
                    }
                } else {
                    if (current_pos > 0) {
                        fseek(file, -current_pos - (ignore_cr ? 2 : 1), SEEK_CUR);
                        current_pos = 0;
                        ignore_cr = 0;
                        ch = fgetc(file);
                        if (ch == '\r') {
                            ch = fgetc(file);
                        }
                        continue;
                    }
                }
                ignore_cr = 0;
            }

            if (found) {
                shared[i] = 1;
                shared[FileCount] = 1;
            }
            fclose(file);
            exit(OK);
        }
    }

    for (int i = 0; i < FileCount; i++) wait(NULL);

    *FlagFound = shared[FileCount];
    memcpy(FoundIn, shared, FileCount * sizeof(char));

    shmdt(shared);
    shmctl(shm_id, IPC_RMID, NULL);
    free(processed_pattern);
    return OK;
}

status_code CheckN(const char *N) {
    if (!N) {
        return PonterNullError;
    }

    int i, len = strlen(N);

    for (i = 4; i < len; ++i) {
        if (!isdigit(N[i])) {
            return NumberError;
        }
    }

    return OK;
}

status_code String_To_uint32_t(const char *str, u_int32_t *result) {
    if (!str || !result) {
        return PonterNullError;
    }

    char *endptr;
    unsigned long res = strtoul(str, &endptr, 16);

    if (res > UINT32_MAX) {
        return InvalidInput;
    }

    if (*endptr != '\0') {
        return InvalidInput;
    }

    *result = (u_int32_t)res;
    return OK;
}