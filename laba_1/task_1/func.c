#include "func.h"

status_code is_valid_login(const char* login) {
    if (!login) return PointerNullError;
    size_t len = strlen(login);
    if (len == 0 || len > 6) return IncorrectLogin;

    for (size_t i = 0; i < len; i++) {
        if (!isalnum((unsigned char)login[i])) {
            return IncorrectLogin;
        }
    }
    return OK;
}

status_code is_valid_pin(int pin) {
    return (pin >= 0 && pin <= 1000000) ? OK : IncorrectPin;
}

status_code sign_in(const char* login, int pin) {
    status_code status = is_valid_login(login);
    if (status != OK) return status;
    
    status = is_valid_pin(pin);
    if (status != OK) return status;

    FILE* db = fopen("database.txt", "a");
    if (!db) return OpenFileError;
    
    fprintf(db, "%s %d\n", login, pin);
    fclose(db);
    return OK;
}

status_code log_in(const char* login, int pin) {
    status_code status = is_valid_login(login);
    if (status != OK) return status;
    
    status = is_valid_pin(pin);
    if (status != OK) return status;

    FILE* db = fopen("database.txt", "r");
    if (!db) return OpenFileError;

    char file_login[7];
    int file_pin;
    int found = 0;

    while (fscanf(db, "%6s %d", file_login, &file_pin) == 2) {
        if (file_pin == pin && strcmp(file_login, login) == 0) {
            found = 1;
            break;
        }
    }

    fclose(db);
    return found ? OK : UndefinedUser;
}

status_code user_exists(const char* username) {
    if (!username) return PointerNullError;
    
    FILE* db = fopen("database.txt", "r");
    if (!db) return OpenFileError;
    
    char file_username[7];
    int pin;
    int found = 0;
    
    while (fscanf(db, "%6s %d", file_username, &pin) == 2) {
        if (strcmp(file_username, username) == 0) {
            found = 1;
            break;
        }
    }
    
    fclose(db);
    return found ? OK : UndefinedUser;
}

status_code add_sanction(SanctionsList* list, const char* username, int max_requests, const char* current_user) {
    if (!list || !username || !current_user || max_requests <= 0) {
        return InputError;
    }

    if (strcmp(username, current_user) == 0) {
        return InputError;
    }

    status_code user_status = user_exists(username);
    if (user_status != OK) {
        return user_status;
    }

    for (int i = 0; i < list->sanctions_count; i++) {
        if (strcmp(list->sanctions[i].username, username) == 0) {
            list->sanctions[i].max_requests = max_requests;
            return OK;
        }
    }

    UserSanction* temp = realloc(list->sanctions, (list->sanctions_count + 1) * sizeof(UserSanction));
    if (!temp) return MemoryAllocationError;
    
    list->sanctions = temp;
    strncpy(list->sanctions[list->sanctions_count].username, username, 6);
    list->sanctions[list->sanctions_count].username[6] = '\0';
    list->sanctions[list->sanctions_count].max_requests = max_requests;
    list->sanctions_count++;
    
    return OK;
}

status_code check_sanction(SanctionsList* list, const char* username, int* current_count) {
    if (!list || !username || !current_count) {
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
    if (!list) return PointerNullError;
    
    FILE* db = fopen("sanctions.txt", "w");
    if (!db) return OpenFileError;
    
    for (int i = 0; i < list->sanctions_count; i++) {
        fprintf(db, "%s %d\n", 
                list->sanctions[i].username,
                list->sanctions[i].max_requests);
    }
    
    fclose(db);
    return OK;
}

status_code load_sanctions(SanctionsList* list) {
    if (!list) return PointerNullError;
    
    FILE* db = fopen("sanctions.txt", "r");
    if (!db) return OpenFileError;
    
    char username[7];
    int max_requests;
    
    while (fscanf(db, "%6s %d", username, &max_requests) == 2) {
        UserSanction* temp = realloc(list->sanctions, (list->sanctions_count + 1) * sizeof(UserSanction));
        if (!temp) {
            fclose(db);
            return MemoryAllocationError;
        }
        list->sanctions = temp;
        strcpy(list->sanctions[list->sanctions_count].username, username);
        list->sanctions[list->sanctions_count].max_requests = max_requests;
        list->sanctions_count++;
    }
    
    fclose(db);
    return OK;
}

void free_sanctions(SanctionsList* list) {
    if (list) {
        free(list->sanctions);
        list->sanctions = NULL;
        list->sanctions_count = 0;
    }
}

status_code get_current_time(char* buffer, size_t buffer_size) {
    if (!buffer) return PointerNullError;
    if (buffer_size < 9) return InputError;
    
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    snprintf(buffer, buffer_size, "%02d:%02d:%02d", 
             tm->tm_hour, tm->tm_min, tm->tm_sec);
    return OK;
}

status_code get_current_date(char* buffer, size_t buffer_size) {
    if (!buffer) return PointerNullError;
    if (buffer_size < 11) return InputError;
    
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    snprintf(buffer, buffer_size, "%02d.%02d.%04d", 
             tm->tm_mday, tm->tm_mon+1, tm->tm_year+1900);
    return OK;
}

int is_leap_year(int year) {
    return (year%4 == 0 && (year%100 != 0 || year%400 == 0));
}

int days_in_month(int month, int year) {
    const int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    return (month == 2 && is_leap_year(year)) ? 29 : days[month-1];
}

status_code parse_date(const char* date_str, struct tm* tm) {
    if (!date_str || !tm) return PointerNullError;
    
    int day, month, year;
    if (sscanf(date_str, "%d.%d.%d", &day, &month, &year) != 3)
        return InvalidDateFormat;
    
    if (year < 1970) return DateBefore1970Error;
    if (month < 1 || month > 12) return InvalidDateValue;
    if (day < 1 || day > days_in_month(month, year)) return InvalidDateValue;
    
    tm->tm_mday = day;
    tm->tm_mon = month-1;
    tm->tm_year = year-1900;
    tm->tm_hour = tm->tm_min = tm->tm_sec = 0;
    tm->tm_isdst = -1;
    
    time_t input_time = mktime(tm);
    if (input_time == -1) return InvalidDateValue;
    
    time_t now = time(NULL);
    if (difftime(input_time, now) > 0) return DateInFuture;
    
    return OK;
}

status_code howmuch(const char* time_str, const char* flag, char** result) {
    if (!time_str || !flag || !result) return PointerNullError;

    struct tm tm = {0};
    status_code parse_status = parse_date(time_str, &tm);
    if (parse_status != OK) return parse_status;

    time_t t1 = mktime(&tm);
    time_t t2 = time(NULL);
    double diff = difftime(t2, t1);

    const char* format = NULL;
    double value = 0.0;

    if (strcmp(flag, "-s") == 0) {
        format = "Прошло секунд: %.0f";
        value = diff;
    } 
    else if (strcmp(flag, "-m") == 0) {
        format = "Прошло минут: %.1f";
        value = diff / 60.0;
    }
    else if (strcmp(flag, "-h") == 0) {
        format = "Прошло часов: %.1f";
        value = diff / 3600.0;
    }
    else if (strcmp(flag, "-y") == 0) {
        format = "Прошло лет: %.1f";
        value = diff / (3600.0 * 24 * 365.25);
    }
    else return InvalidFlag;

    int needed_size = snprintf(NULL, 0, format, value) + 1;
    *result = malloc(needed_size);
    if (!*result) return MemoryAllocationError;
    
    snprintf(*result, needed_size, format, value);
    return OK;
}

status_code read_line(char** line) {
    if (!line) return PointerNullError;
    
    size_t len = 0;
    size_t read = getline(line, &len, stdin);
    
    if (read == -1) return InputError;
    if ((*line)[read - 1] == '\n') (*line)[read - 1] = '\0';
    
    return OK;
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