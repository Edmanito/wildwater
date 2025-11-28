CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -Iinclude

SRC = \
    src/main.c \
    src/csv.c \
    src/avl.c \
    src/tree.c \
    src/utils.c

EXEC_DIR = exec
EXEC = $(EXEC_DIR)/Wildwater

.PHONY: all compile run clean

all: $(EXEC)

$(EXEC): $(SRC)
	@echo "Compilation..."
	@mkdir -p $(EXEC_DIR)
	@$(CC) $(CFLAGS) -o $(EXEC) $(SRC)



#Nettoyage
clean:
	@echo "Nettoyage..."
	@rm -f $(EXEC)


#Compilation
compile: clean all


#Execution
run: all
	@echo "Exécution..."
	@./$(EXEC)

