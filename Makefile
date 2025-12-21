CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -std=c99 -g -Iinclude -D_POSIX_C_SOURCE=200809L
LDFLAGS = -lm

# --- COULEURS ---
B = \033[0;34m
G = \033[0;32m
Y = \033[1;33m
N = \033[0m

##############################################
# Sources et Headers
##############################################

SRC = \
    src/main.c \
    src/csv.c \
    src/avl.c \
    src/arbre.c \
    src/utiles.c \
    src/histo_src.c \
    src/histo_real.c \
    src/histo_all.c \
    src/leaks.c

HEADERS = \
    include/csv.h \
    include/avl.h \
    include/arbre.h \
    include/utiles.h \
    include/histo_src.h \
    include/histo_real.h \
    include/histo_all.h \
    include/leaks.h

##############################################
# Chemins
##############################################

EXEC_DIR = exec
EXEC     = $(EXEC_DIR)/Wildwater
SCRIPT   = scripts/script.sh
DATA     = exemple/data.csv

##############################################
# Phony
##############################################

.PHONY: all logo clean run histo-max histo-src histo-real histo-all leaks

##############################################
# Compilation DIRECTE (Sans .o)
##############################################

all: logo $(EXEC)

logo:
	@echo "$(B)"
	@echo " _       _ _ _     _       _       _            "
	@echo "| |     | (_) |   | |     | |     | |           "
	@echo "| |  _  | |_| | __| |_ ___| | __ _| |_ ___ _ __ "
	@echo "| | | | | | | |/ _\` \ \ /\ / / _\` | __/ _ \ '__|"
	@echo "| |/| |/ /| | | (_| |\ V  V / (_| | ||  __/ |   "
	@echo "|__/|__/  |_|_|\__,_| \_/\_/ \__,_|\__\___|_|   "
	@echo "$(N)"

# Ici, on compile TOUT d'un coup : SRC -> EXEC
$(EXEC): $(SRC) $(HEADERS)
	@mkdir -p $(EXEC_DIR)
	@echo "$(B)[CC]   Compilation complète du projet...$(N)"
	@$(CC) $(CFLAGS) -o $(EXEC) $(SRC) $(LDFLAGS)
	@echo "$(G)[OK]   Wildwater est prêt : $(EXEC)$(N)"

##############################################
# Nettoyage
##############################################

clean:
	@echo "$(Y)[CLEAN] Nettoyage...$(N)"
	@rm -f $(EXEC)
	@rm -rf data/histo/*.csv data/histo/*.png 
	@rm -rf data/dat/*.dat 
	@rm -rf data/tmp/* data/leaks/*.csv data/leaks/*.dat
	@echo "$(G)[OK]   Terminé.$(N)"

##############################################
# Exécution simple
##############################################

run: $(EXEC)
	@./$(EXEC)

##############################################
# Commandes via Script
##############################################

histo-max: $(EXEC)
	@bash $(SCRIPT) $(DATA) histo max

histo-src: $(EXEC)
	@bash $(SCRIPT) $(DATA) histo src

histo-real: $(EXEC)
	@bash $(SCRIPT) $(DATA) histo real

histo-all: $(EXEC)
	@bash $(SCRIPT) $(DATA) histo all

leaks: $(EXEC)
ifndef ID
	$(error Usage: make leaks ID="<ID_USINE>")
endif
	@bash $(SCRIPT) $(DATA) leaks "$(ID)"