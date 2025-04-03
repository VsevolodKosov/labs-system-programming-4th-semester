#ifndef FUNC_H
#define FUNC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

typedef enum {
    OK,
    PointerNullError,
    InputError,
    IncorrectLogin,
    IncorrectPin,
    UndefinedUser,
    MemoryAllocationError,
    OpenFileError,
    SanctionLimitExceeded,
    InvalidDateFormat,
    InvalidDateValue,
    DateInFuture,
    InvalidFlag,
    DateBefore1970Error
} status_code;

typedef struct {
    char username[7];
    int max_requests;
} UserSanction;

typedef struct {
    UserSanction* sanctions;
    int sanctions_count;
} SanctionsList;

status_code is_valid_login(const char* login);
status_code is_valid_pin(int pin);
status_code user_exists(const char* username);

status_code sign_in(const char* login, int pin);
status_code log_in(const char* login, int pin);

status_code add_sanction(SanctionsList* list, const char* username, int max_requests, const char* current_user);
status_code check_sanction(SanctionsList* list, const char* username, int* current_count);
status_code save_sanctions(const SanctionsList* list);
status_code load_sanctions(SanctionsList* list);
void free_sanctions(SanctionsList* list);

status_code get_current_time(char* buffer, size_t buffer_size);
status_code get_current_date(char* buffer, size_t buffer_size);
status_code howmuch(const char* time_str, const char* flag, char** result);
int is_leap_year(int year);
int days_in_month(int month, int year);
status_code parse_date(const char* date_str, struct tm* tm_info);

status_code read_line(char** line);
void print_menu();

#endif