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
    SanctionLimitExceeded
} status_code;

typedef struct {
    char username[7];
    int max_requests;
} UserSanction;

typedef struct {
    UserSanction* sanctions;
    int sanctions_count;
} SanctionsList;

status_code is_valid_login(const char* login, size_t length_login);
status_code is_valid_pin(int pin);
status_code sign_in(char** login, size_t length_login, int pin);
status_code log_in(char** login, size_t length_login, int pin);
status_code user_exists(const char* username);

status_code add_sanction(SanctionsList* list, const char* username, int max_requests, const char* current_user);
status_code check_sanction(SanctionsList* list, const char* username, int* current_count);
status_code save_sanctions(const SanctionsList* list);
status_code load_sanctions(SanctionsList* list);
void clear_sanctions_file();
void free_sanctions(SanctionsList* list);

void print_menu();
void get_current_time();
void get_current_date();
int is_leap_year(int year);
int days_in_month(int month, int year);
int parse_date(const char* date_str, struct tm* tm_info);
void howmuch(const char* time_str, const char* flag);

#endif