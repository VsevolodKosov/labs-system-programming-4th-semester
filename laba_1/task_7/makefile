CC = gcc
CFLAGS = -Wall -g

SRC = func.c main.c
EXEC = my_ls

all: $(EXEC)

$(EXEC): $(SRC)
	@$(CC) $(CFLAGS) $(SRC) -o $(EXEC)

clean:
	@rm -f $(EXEC)

debug: CFLAGS += -DDEBUG
debug: $(EXEC)

valgrind: $(EXEC)
	@echo "Запуск с Valgrind..."
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(EXEC) $(ARGS)
