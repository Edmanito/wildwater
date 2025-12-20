CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -Iinclude

##############################################
# Sources C (exécutable principal)
##############################################

SRC = \
	src/main.c \
	src/csv.c \
	src/avl.c \
	src/arbre.c \
	src/utiles.c \
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
# Exécutable principal
##############################################

EXEC_DIR = exec
EXEC     = $(EXEC_DIR)/Wildwater

##############################################
# Binaires outils histogrammes (C)
##############################################

HISTO_SRC_SRC  = src/histo_src.c
HISTO_SRC_HDR  = include/histo_src.h
HISTO_SRC_EXEC = $(EXEC_DIR)/histo_src

HISTO_REAL_SRC  = src/histo_real.c
HISTO_REAL_HDR  = include/histo_real.h
HISTO_REAL_EXEC = $(EXEC_DIR)/histo_real

HISTO_ALL_SRC  = src/histo_all.c
HISTO_ALL_HDR  = include/histo_all.h
HISTO_ALL_EXEC = $(EXEC_DIR)/histo_all

##############################################
# Script & données
##############################################

SCRIPT = scripts/script.sh
DATA   = exemple/data.csv

##############################################
# Phony
##############################################

.PHONY: all clean run \
        histo-max histo-src histo-real histo-all leaks \
        histo-src-bin histo-real-bin histo-all-bin

##############################################
# Compilation (principal)
##############################################

all: $(EXEC)

$(EXEC): $(SRC) $(HEADERS)
	@echo "Compilation Wildwater..."
	@mkdir -p $(EXEC_DIR)
	@$(CC) $(CFLAGS) -o $(EXEC) $(SRC)

##############################################
# Compilation outils histogrammes (binaires)
##############################################

histo-src-bin: $(HISTO_SRC_EXEC)

$(HISTO_SRC_EXEC): $(HISTO_SRC_SRC) $(HISTO_SRC_HDR)
	@echo "Compilation histo_src..."
	@mkdir -p $(EXEC_DIR)
	@$(CC) $(CFLAGS) -o $(HISTO_SRC_EXEC) $(HISTO_SRC_SRC)

histo-real-bin: $(HISTO_REAL_EXEC)

$(HISTO_REAL_EXEC): $(HISTO_REAL_SRC) $(HISTO_REAL_HDR)
	@echo "Compilation histo_real..."
	@mkdir -p $(EXEC_DIR)
	@$(CC) $(CFLAGS) -o $(HISTO_REAL_EXEC) $(HISTO_REAL_SRC)

histo-all-bin: $(HISTO_ALL_EXEC)

$(HISTO_ALL_EXEC): $(HISTO_ALL_SRC) $(HISTO_ALL_HDR)
	@echo "Compilation histo_all..."
	@mkdir -p $(EXEC_DIR)
	@$(CC) $(CFLAGS) -o $(HISTO_ALL_EXEC) $(HISTO_ALL_SRC)

##############################################
# Nettoyage
##############################################

clean:
	@echo "Nettoyage du projet..."
	@rm -f $(EXEC) $(HISTO_SRC_EXEC) $(HISTO_REAL_EXEC) $(HISTO_ALL_EXEC)
	@rm -rf data/histo/*.csv data/histo/*.png data/dat/*.dat data/tmp/* data/leaks/*.csv

##############################################
# Exécution C (si besoin)
##############################################

run: $(EXEC)
	@./$(EXEC)

##############################################
# Histogrammes
##############################################

histo-max: $(EXEC)
	@echo "Histogramme MAX"
	@bash $(SCRIPT) $(DATA) histo max

histo-src: histo-max histo-src-bin
	@echo "Histogramme SRC"
	@bash $(SCRIPT) $(DATA) histo src

histo-real: histo-max histo-real-bin
	@echo "Histogramme REAL"
	@bash $(SCRIPT) $(DATA) histo real

histo-all: histo-max histo-src histo-real histo-all-bin
	@echo "Histogramme ALL"
	@bash $(SCRIPT) $(DATA) histo all

##############################################
# Leaks
##############################################

# Utilisation :
# make leaks ID="Facility complex #RH400057F"
leaks: $(EXEC)
ifndef ID
	$(error Usage: make leaks ID="<ID_USINE>")
endif
	@echo "Calcul des fuites pour $(ID)"
	@bash $(SCRIPT) $(DATA) leaks $(ID)

