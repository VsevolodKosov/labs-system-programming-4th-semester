#include "func.h"

int main(int argc, char const *argv[]) {
    int i, N, count;
    char *AnswerString, FlagFound;
    u_int64_t XorResult;

    if (argc < 3) {
        printf("Неверный ввод. Введите флаг и название файла\n");
        return InvalidInput;
    }

    for (i = 0; i < argc; i++) {
        if (argv[i] == NULL) {
            printf("Неверный ввод. Введите флаг и название файла\n");
            return InvalidInput;
        }
    }

    if (strncmp(argv[argc - 1], "xor", 3) == 0) {
        N = strtol(argv[argc - 1] + 3, NULL, 10);
        if (N < 2 || N > 6) {
            printf("Неверное значение N. N должно быть >= 2 и <= 6\n");
            return InvalidInput;
        }

        // Xor.bin: 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10
        for (i = 1; i < argc - 1; ++i) {
            switch(Xor(argv[i], N, &XorResult)) {
                case MemoryAllocationError:
                    printf("Ошибка выделения памяти\n");
                    return MemoryAllocationError;
                case OpenFileError:
                    printf("Проблема с файлом (%s)\n", argv[i]);
                    break;
                default:
                    printf("Результат Xor [%s]: %lu\n", argv[i], XorResult);
                    break;
            }
        }
    } else if (strcmp(argv[argc - 2], "mask") == 0) {
        for (i = 1; i < argc - 2; ++i) {
            switch (Mask(argv[i], argv[argc - 1], &count)) {
                case InvalidInput:
                    printf("Неверный ввод. Ошибка с %s: \n", argv[argc - 1]);
                    return InvalidInput;
                case MemoryAllocationError:
                    printf("Ошибка выделения памяти\n");
                    return MemoryAllocationError;
                case OpenFileError:
                    printf("Проблема с файлом (%s)\n", argv[i]);
                    break;
                default:
                    printf("Результат Mask [%s]: %d\n", argv[i], count);
                    break;
            }
        }
    } else if (strncmp(argv[argc - 1], "copy", 4) == 0) {
        if (CheckN(argv[argc - 1]) != OK) {
            printf("Ошибка с числом\n");
        } else {
            N = strtol(argv[argc - 1] + 4, NULL, 10);
            if (N < 0 || N > 10) {
                printf("Слишком много копий\n");
                return InvalidInput;
            }

            for (i = 1; i < argc - 1; ++i) {
                switch (CopyN(argv[i], &N)) {
                    case MemoryAllocationError:
                        printf("Ошибка выделения памяти. %s не обработан\n", argv[i]);
                        return MemoryAllocationError;
                    case OpenFileError:
                        printf("Проблема с файлом. %s не обработан\n", argv[i]);
                        break;
                    case ErrorPid:
                        printf("Проблема с pid. %s не обработан\n", argv[i]);
                        return ErrorPid;
                    default:
                        printf("Файл %s успешно обработан\n", argv[i]);
                        break;
                }
            }
        }
    } else if (strncmp(argv[argc - 2], "find", 4) == 0) {
        AnswerString = (char *)malloc(sizeof(char) * (argc - 3));
        if (!AnswerString) {
            printf("Ошибка выделения памяти\n");
            return MemoryAllocationError;
        }

        N = argc - 3;
        switch (FindString((const char **)argv + 1, N, argv[argc - 1], AnswerString, &FlagFound)) {
            case MemoryAllocationError:
                printf("Ошибка выделения памяти\n");
                free(AnswerString);
                break;
            case OpenFileError:
                printf("Проблема с файлом\n");
                free(AnswerString);
                break;
            case ErrorPid:
                printf("Проблема с pid\n");
                free(AnswerString);
                break;
            default:
                printf("Файлы успешно обработаны\n");
                for (i = 0; i < argc - 3; ++i) {
                    if (FlagFound && AnswerString[i]) {
                        printf("В файле: %s строка найдена\n", argv[i + 1]);
                    } else {
                        printf("В файле: %s строка НЕ найдена\n", argv[i + 1]);
                    }
                }
                break;
        }
        free(AnswerString);
    }

    return 0;
}