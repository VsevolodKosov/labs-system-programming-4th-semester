#include "func.h"

status_code is_valid_login(const char* login, size_t length_login) {
    if (login == NULL) return PointerNullError;
    size_t actual_length = strlen(login);
    if (actual_length == 0 || actual_length > 6) return IncorrectLogin;

    for (size_t i = 0; i < actual_length; i++) {
        if (!isalnum((unsigned char)login[i])) {
            return IncorrectLogin;
        }
    }
    return OK;
}

status_code is_valid_pin(int pin) {
    return (pin >= 0 && pin <= 1000000) ? OK : IncorrectPin;
}

status_code sign_in(char** login, size_t length_login, int pin) {
    if (login == NULL || *login == NULL) return PointerNullError;

    status_code status = is_valid_login(*login, length_login);
    if (status != OK) {
        free(*login);
        *login = NULL;
        return status;
    }
    
    status = is_valid_pin(pin);
    if (status != OK) {
        free(*login);
        *login = NULL;
        return status;
    }

    FILE* data_base = fopen("database.txt", "a");
    if (data_base == NULL) {
        free(*login);
        *login = NULL;
        return OpenFileError;
    }
    
    fprintf(data_base, "%s %d\n", *login, pin);
    fclose(data_base);
    return OK;
}

status_code log_in(char** login, size_t length_login, int pin) {
    if (login == NULL || *login == NULL) return PointerNullError;

    status_code status = is_valid_login(*login, length_login);
    if (status != OK) {
        free(*login);
        *login = NULL;
        return status;
    }
    
    status = is_valid_pin(pin);
    if (status != OK) {
        free(*login);
        *login = NULL;
        return status;
    }

    FILE* data_base = fopen("database.txt", "r");
    if (data_base == NULL) {
        free(*login);
        *login = NULL;
        return OpenFileError;
    }

    char file_login[7];
    int file_pin;
    int found = 0;

    while (fscanf(data_base, "%6s %d", file_login, &file_pin) == 2) {
        if (file_pin == pin && strcmp(file_login, *login) == 0) {
            found = 1;
            break;
        }
    }

    fclose(data_base);

    if (!found) {
        free(*login);
        *login = NULL;
        return UndefinedUser;
    }
    return OK;
}

status_code user_exists(const char* username) {
    if (username == NULL) return PointerNullError;
    
    FILE* data_base = fopen("database.txt", "r");
    if (data_base == NULL) return OpenFileError;
    
    char file_username[7];
    int pin;
    int found = 0;
    
    while (fscanf(data_base, "%6s %d", file_username, &pin) == 2) {
        if (strcmp(file_username, username) == 0) {
            found = 1;
            break;
        }
    }
    
    fclose(data_base);
    return found ? OK : UndefinedUser;
}

status_code add_sanction(SanctionsList* list, const char* username, int max_requests, const char* current_user) {
    if (list == NULL || username == NULL || max_requests <= 0) {
        return InputError;
    }

    if (strcmp(username, current_user) == 0) {
        printf("Нельзя накладывать ограничения на себя\n");
        return InputError;
    }

    status_code user_status = user_exists(username);
    if (user_status != OK) {
        printf("Пользователь не найден\n");
        return user_status;
    }

    printf("Введите 12345 для подтверждения: ");
    int confirmation;
    if (scanf("%d", &confirmation) != 1 || confirmation != 12345) {
        while (getchar() != '\n');
        printf("Отмена операции\n");
        return InputError;
    }
    while (getchar() != '\n');

    for (int i = 0; i < list->sanctions_count; i++) {
        if (strcmp(list->sanctions[i].username, username) == 0) {
            list->sanctions[i].max_requests = max_requests;
            printf("Лимит обновлен для %s\n", username);
            return OK;
        }
    }

    UserSanction* temp = realloc(list->sanctions, 
                               (list->sanctions_count + 1) * sizeof(UserSanction));
    if (temp == NULL) return MemoryAllocationError;
    
    list->sanctions = temp;
    strncpy(list->sanctions[list->sanctions_count].username, username, 6);
    list->sanctions[list->sanctions_count].username[6] = '\0';
    list->sanctions[list->sanctions_count].max_requests = max_requests;
    list->sanctions_count++;
    
    printf("Ограничения установлены для %s\n", username);
    return OK;
}

status_code check_sanction(SanctionsList* list, const char* username, int* current_count) {
    if (list == NULL || username == NULL || current_count == NULL) {
        return InputError;
    }

    for (int i = 0; i < list->sanctions_count; i++) {
        if (strcmp(list->sanctions[i].username, username) == 0) {
            (*current_count)++;
            if (*current_count > list->sanctions[i].max_requests) {
                return SanctionLimitExceeded;
            }
            return OK;
        }
    }
    return OK;
}

status_code save_sanctions(const SanctionsList* list) {
    if (list == NULL) return PointerNullError;
    
    FILE* data_base = fopen("sanctions.txt", "w");
    if (data_base == NULL) return OpenFileError;
    
    for (int i = 0; i < list->sanctions_count; i++) {
        fprintf(data_base, "%s %d\n", 
                list->sanctions[i].username,
                list->sanctions[i].max_requests);
    }
    
    fclose(data_base);
    return OK;
}

status_code load_sanctions(SanctionsList* list) {
    if (list == NULL) return PointerNullError;
    
    FILE* data_base = fopen("sanctions.txt", "r");
    if (data_base == NULL) return OpenFileError;
    
    char username[7];
    int max_requests;
    
    while (fscanf(data_base, "%6s %d", username, &max_requests) == 2) {
        UserSanction* temp = realloc(list->sanctions, 
                                   (list->sanctions_count + 1) * sizeof(UserSanction));
        if (temp == NULL) {
            fclose(data_base);
            return MemoryAllocationError;
        }
        
        list->sanctions = temp;
        strcpy(list->sanctions[list->sanctions_count].username, username);
        list->sanctions[list->sanctions_count].max_requests = max_requests;
        list->sanctions_count++;
    }
    
    fclose(data_base);
    return OK;
}

void free_sanctions(SanctionsList* list) {
    if (list != NULL) {
        free(list->sanctions);
        list->sanctions = NULL;
        list->sanctions_count = 0;
    }
}

void clear_sanctions_file() {
    FILE* file = fopen("sanctions.txt", "w");
    if (file != NULL) {
        fclose(file);
    }
}

void print_menu() {
    printf("------------------------------------------------\n");
    printf("1. Time - текущее время\n");
    printf("2. Date - текущая дата\n");
    printf("3. Howmuch <дата> <флаг>\n");
    printf("4. Logout - выход\n");
    printf("5. Sanctions <user> <limit>\n");
    printf("------------------------------------------------\n");
    printf("Выбор: ");
}

void get_current_time() {
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    printf("Текущее время: %02d:%02d:%02d\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
}

void get_current_date() {
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    printf("Текущая дата: %02d.%02d.%04d\n", tm->tm_mday, tm->tm_mon+1, tm->tm_year+1900);
}

int is_leap_year(int year) {
    return (year%4 == 0 && (year%100 != 0 || year%400 == 0));
}

int days_in_month(int month, int year) {
    const int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    return (month == 2 && is_leap_year(year)) ? 29 : days[month-1];
}

int parse_date(const char* date_str, struct tm* tm) {
    int day, month, year;
    if (sscanf(date_str, "%d.%d.%d", &day, &month, &year) != 3)
        return -1;
    
    if (year < 1970 || month < 1 || month > 12 || day < 1 || day > days_in_month(month, year))
        return -1;
    
    tm->tm_mday = day;
    tm->tm_mon = month-1;
    tm->tm_year = year-1900;
    tm->tm_hour = tm->tm_min = tm->tm_sec = 0;
    tm->tm_isdst = -1;
    
    time_t input_time = mktime(tm);
    time_t now = time(NULL);
    
    if (input_time == -1) return -1;
    if (difftime(input_time, now) > 0) return -2;
    
    return 0;
}

void howmuch(const char* time_str, const char* flag) {
    struct tm tm = {0};
    int parse_result = parse_date(time_str, &tm);
    
    if (parse_result == -1) {
        printf("Неверный формат даты или несуществующая дата\n");
        return;
    }
    else if (parse_result == -2) {
        printf("Дата не может быть в будущем\n");
        return;
    }
    
    time_t t1 = mktime(&tm);
    time_t t2 = time(NULL);
    double diff = difftime(t2, t1);
    
    if (strcmp(flag, "-s") == 0) {
        printf("Прошло секунд: %.0f\n", diff);
    }
    else if (strcmp(flag, "-m") == 0) {
        printf("Прошло минут: %.1f\n", diff/60);
    }
    else if (strcmp(flag, "-h") == 0) {
        printf("Прошло часов: %.1f\n", diff/3600);
    }
    else if (strcmp(flag, "-y") == 0) {
        printf("Прошло лет: %.1f\n", diff/(3600*24*365.25));
    }
    else {
        printf("Неизвестный флаг\n");
    }
}