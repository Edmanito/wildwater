CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -Iinclude

SRC = \
    src/main.c \
    src/csv.c \
    src/avl.c \
    src/arbre.c \
    src/utiles.c

HEADERS = \
    include/csv.h \
    include/avl.h \
    include/arbre.h \
    include/utiles.h

EXEC_DIR = exec
EXEC = $(EXEC_DIR)/Wildwater

.PHONY: all compile run clean

all: $(EXEC)

$(EXEC): $(SRC) $(HEADERS)
	@echo "Compilation..."
	@mkdir -p $(EXEC_DIR)
	@$(CC) $(CFLAGS) -o $(EXEC) $(SRC)

clean:
	@echo "Nettoyage..."
	@rm -f $(EXEC)

compile: clean all

run: all
	@echo "Exécution..."
	@./$(EXEC)
