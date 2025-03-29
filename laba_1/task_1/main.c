#include "func.h"

int main() {
    SanctionsList sanctions = {NULL, 0};
    
    while (1) {
        printf("1 - Вход, 2 - Регистрация, 3 - Выход из программы: ");
        
        char input[100];
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
            clear_sanctions_file();
            free_sanctions(&sanctions);
            break;
        }

        char* login = NULL;
        size_t len = 0;
        printf("Введите логин (до 6 символов): ");
        if (getline(&login, &len, stdin) == -1) {
            printf("Ошибка ввода логина\n");
            continue;
        }
        
        size_t login_len = strlen(login);
        if (login_len > 0 && login[login_len-1] == '\n') {
            login[login_len-1] = '\0';
            login_len--;
        }
        
        // Проверка логина
        int invalid_login = 0;
        if (login_len == 0 || login_len > 6) {
            printf("Некорректный логин! Должен быть от 1 до 6 символов\n");
            invalid_login = 1;
        }
        
        if (!invalid_login) {
            for (size_t i = 0; i < login_len; i++) {
                if (!isalnum((unsigned char)login[i])) {
                    printf("Некорректный логин! Только буквы и цифры\n");
                    invalid_login = 1;
                    break;
                }
            }
        }
        
        if (invalid_login) {
            free(login);
            continue;
        }
        
        if (choice == 2) {
            FILE* db = fopen("database.txt", "r");
            if (db) {
                char user[7];
                int file_pin;
                int user_exists = 0;
                while (fscanf(db, "%6s %d", user, &file_pin) == 2) {
                    if (strcmp(user, login) == 0) {
                        user_exists = 1;
                        break;
                    }
                }
                fclose(db);
                if (user_exists) {
                    printf("Ошибка: Пользователь '%s' уже существует\n", login);
                    free(login);
                    continue;
                }
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

        if (pin < 0 || pin > 1000000) {
            printf("Некорректный PIN! Должен быть от 0 до 1000000\n");
            free(login);
            continue;
        }

        status_code res;
        if (choice == 1) {
            res = log_in(&login, login_len, pin);
        } else {
            res = sign_in(&login, login_len, pin);
        }
        
        if (res != OK) {
            if (res == UndefinedUser) {
                printf("Ошибка: Пользователь не найден\n");
            } else if (res == IncorrectLogin) {
                printf("Ошибка: Неверный логин\n");
            } else if (res == IncorrectPin) {
                printf("Ошибка: Неверный PIN\n");
            } else if (res == OpenFileError) {
                printf("Ошибка: Проблема с доступом к базе данных\n");
            } else {
                printf("Неизвестная ошибка авторизации\n");
            }
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
                case 1: 
                    get_current_time();
                    break;
                    
                case 2:  
                    get_current_date();
                    break;
                    
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
                    howmuch(date, flag);
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
                    
                    if (strcmp(username, login) == 0) {
                        printf("Ошибка: Нельзя накладывать ограничения на себя\n");
                        continue;
                    }
                    
                    status_code add_status = add_sanction(&sanctions, username, limit, login);
                    if (add_status != OK) {
                        printf("Ошибка установки ограничений\n");
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