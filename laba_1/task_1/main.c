#include "func.h"

int main() {
    SanctionsList sanctions = {NULL, 0};
    
    while (1) {
        printf("1 - Вход, 2 - Регистрация, 3 - Выход из программы: ");
        
        char input[10];
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Ошибка ввода\n");
            continue;
        }
        
        input[strcspn(input, "\n")] = '\0';
        
        if (strlen(input) != 1 || input[0] < '1' || input[0] > '3') {
            printf("Неверный ввод. Введите 1, 2 или 3.\n");
            continue;
        }
        
        int choice = input[0] - '0';
        
        if (choice == 3) {
            printf("Выход из программы\n");
            FILE* file = fopen("sanctions.txt", "w");
            if (file) fclose(file);
            free_sanctions(&sanctions);
            break;
        }

        char* login = NULL;
        printf("Введите логин (до 6 символов): ");
        if (read_line(&login) != OK) {
            printf("Ошибка ввода логина\n");
            continue;
        }

        status_code login_status = is_valid_login(login);
        if (login_status != OK) {
            printf("Некорректный логин! Должен быть от 1 до 6 символов (буквы и цифры)\n");
            free(login);
            continue;
        }

        if (choice == 2) {
            if (user_exists(login) == OK) {
                printf("Ошибка: Пользователь '%s' уже существует\n", login);
                free(login);
                continue;
            }
        }

        int pin;
        printf("Введите PIN-код (0-1000000): ");
        if (scanf("%d", &pin) != 1) {
            printf("Ошибка: Введите целое число для PIN\n");
            free(login);
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n');

        status_code pin_status = is_valid_pin(pin);
        if (pin_status != OK) {
            printf("Некорректный PIN! Должен быть от 0 до 1000000\n");
            free(login);
            continue;
        }

        status_code auth_status;
        if (choice == 1) {
            auth_status = log_in(login, pin);
        } else {
            auth_status = sign_in(login, pin);
        }

        switch(auth_status) {
            case OK:
                printf("Успешная авторизация\n");
                break;
            case UndefinedUser:
                printf("Ошибка: Пользователь не найден\n");
                free(login);
                continue;
            case IncorrectLogin:
                printf("Ошибка: Неверный логин\n");
                free(login);
                continue;
            case IncorrectPin:
                printf("Ошибка: Неверный PIN\n");
                free(login);
                continue;
            case OpenFileError:
                printf("Ошибка: Проблема с доступом к базе данных\n");
                free(login);
                continue;
            default:
                printf("Неизвестная ошибка авторизации\n");
                free(login);
                continue;
        }

        if (load_sanctions(&sanctions) != OK) {
            printf("Не удалось загрузить ограничения\n");
        }

        int request_count = 0;
        int running = 1;
        
        while (running) {
            print_menu();
            
            if (fgets(input, sizeof(input), stdin) == NULL) {
                printf("Ошибка ввода команды\n");
                continue;
            }
            
            input[strcspn(input, "\n")] = '\0';
            
            if (strlen(input) != 1 || input[0] < '1' || input[0] > '5') {
                printf("Неверная команда. Введите число от 1 до 5.\n");
                continue;
            }
            
            int menu_choice = input[0] - '0';
            
            if (check_sanction(&sanctions, login, &request_count) == SanctionLimitExceeded) {
                printf("Превышен лимит запросов! Возврат в меню авторизации.\n");
                break;
            }
            
            switch (menu_choice) {
                case 1: {
                    char time_buf[9];
                    if (get_current_time(time_buf, sizeof(time_buf)) == OK) {
                        printf("Текущее время: %s\n", time_buf);
                    } else {
                        printf("Ошибка получения времени\n");
                    }
                    break;
                }
                    
                case 2: {
                    char date_buf[11];
                    if (get_current_date(date_buf, sizeof(date_buf)) == OK) {
                        printf("Текущая дата: %s\n", date_buf);
                    } else {
                        printf("Ошибка получения даты\n");
                    }
                    break;
                }
                    
                case 3: {
                    char date[11], flag[3];
                    printf("Введите дату (дд.мм.гггг): ");
                    if (scanf("%10s", date) != 1) {
                        printf("Ошибка ввода даты\n");
                        while (getchar() != '\n');
                        continue;
                    }
                    printf("Введите флаг (-s, -m, -h, -y): ");
                    if (scanf("%2s", flag) != 1) {
                        printf("Ошибка ввода флага\n");
                        while (getchar() != '\n');
                        continue;
                    }
                    while (getchar() != '\n');

                    char* result = NULL;
                    status_code howmuch_res = howmuch(date, flag, &result);
                    
                    switch(howmuch_res) {
                        case OK:
                            printf("%s\n", result);
                            break;
                        case InvalidDateFormat:
                            printf("Неверный формат даты. Используйте дд.мм.гггг\n");
                            break;
                        case DateBefore1970Error:
                            printf("К сожалению, даты до 1970 года не обрабатываются программой\n");
                            break;
                        case InvalidDateValue:
                            printf("Неверное значение даты. Проверьте день, месяц и год\n");
                            break;
                        case DateInFuture:
                            printf("Дата не может быть в будущем\n");
                            break;
                        case InvalidFlag:
                            printf("Неверный флаг. Допустимые флаги: -s, -m, -h, -y\n");
                            break;
                        case MemoryAllocationError:
                            printf("Ошибка выделения памяти\n");
                            break;
                        default:
                            printf("Неизвестная ошибка\n");
                    }
                    if (result) free(result);
                    break;
                }
                    
                case 4: 
                    running = 0;
                    printf("Выход в меню авторизации\n");
                    break;
                    
                case 5: {
                    char username[7];
                    int limit;
                    printf("Введите имя пользователя и лимит запросов: ");
                    if (scanf("%6s %d", username, &limit) != 2) {
                        printf("Неверный формат ввода\n");
                        while (getchar() != '\n');
                        continue;
                    }
                    while (getchar() != '\n');
                    
                    printf("Введите 12345 для подтверждения: ");
                    int confirmation;
                    if (scanf("%d", &confirmation) != 1 || confirmation != 12345) {
                        printf("Отмена операции\n");
                        while (getchar() != '\n');
                        continue;
                    }
                    while (getchar() != '\n');

                    status_code add_status = add_sanction(&sanctions, username, limit, login);
                    switch(add_status) {
                        case OK:
                            printf("Ограничения установлены для %s\n", username);
                            break;
                        case InputError:
                            if (strcmp(username, login) == 0) {
                                printf("Ошибка: Нельзя накладывать ограничения на себя\n");
                            } else {
                                printf("Ошибка: Неверные параметры санкции\n");
                            }
                            break;
                        case UndefinedUser:
                            printf("Ошибка: Пользователь не найден\n");
                            break;
                        case MemoryAllocationError:
                            printf("Ошибка выделения памяти\n");
                            break;
                        default:
                            printf("Неизвестная ошибка\n");
                    }
                    break;
                }
            }
        }
        
        save_sanctions(&sanctions);
        free_sanctions(&sanctions);
        free(login);
    }
    return 0;
}