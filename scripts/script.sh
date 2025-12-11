#!/bin/bash

# COULEURS
GREEN="\e[92m"
BLUE="\e[94m"
YELLOW="\e[93m"
RED="\e[91m"
RESET="\e[0m"

# TIMER
start=$(date +%s)

# ARGUMENTS
if [ $# -lt 2 ]; then
    echo -e "${RED}✖️ Usage : $0 <fichier.csv> histo {max|src|real} | leaks <ID>${RESET}"
    exit 1
fi

INPUT="$1"
CMD="$2"
MODE="$3"

if [ ! -f "$INPUT" ]; then
    echo -e "${RED}✖️ Le fichier '$INPUT' n'existe pas.${RESET}"
    exit 1
fi

########################################################
#                      HISTO                           #
########################################################

if [ "$CMD" = "histo" ]; then

    if [ -z "$MODE" ]; then
        echo -e "${RED}✖️ histo nécessite un mode : max | src | real${RESET}"
        exit 1
    fi

    if [ "$MODE" != "max" ] && [ "$MODE" != "src" ] && [ "$MODE" != "real" ]; then
        echo -e "${RED}✖️ Mode histo inconnu : '$MODE'${RESET}"
        exit 1
    fi

    ########################################################
    #                    HISTO MAX                         #
    ########################################################
    if [ "$MODE" = "max" ]; then
        echo -e "${BLUE}┌──────────────────────────────────────────┐${RESET}"
        echo -e "${BLUE}│   ${GREEN}Histogramme : Mode MAX${BLUE}                      │${RESET}"
        echo -e "${BLUE}└──────────────────────────────────────────┘${RESET}"

        OUTFILE="histo_max.dat"

        echo -e "${YELLOW}➤ Lecture du fichier...${RESET}"
        echo "identifier;max_volume(k.m3.year-1)" > "$OUTFILE"

        echo -e "${YELLOW}➤ Extraction des capacités maximales...${RESET}"
        grep ";-;" "$INPUT" | grep -E "Facility|Factory" \
            | awk -F';' '{print $2 ";" $4}' \
            | sort -r >> "$OUTFILE"

        echo -e "${GREEN}✔️  Fichier créé :${RESET} $OUTFILE"
        echo -e "${BLUE}⏳ Temps : $(( $(date +%s) - start ))s${RESET}"
        echo -e "${BLUE}────────────────────────────────────────────${RESET}"
        exit 0
    fi

    ########################################################
    #                    HISTO SRC                         #
    ########################################################
    if [ "$MODE" = "src" ]; then
        echo -e "${BLUE}┌──────────────────────────────────────────┐${RESET}"
        echo -e "${BLUE}│   ${GREEN}Histogramme : Mode SRC${BLUE}                      │${RESET}"
        echo -e "${BLUE}└──────────────────────────────────────────┘${RESET}"

        OUTFILE="histo_src.dat"
        echo "identifier;source_volume(k.m3.year-1)" > "$OUTFILE"

        echo -e "${YELLOW}➤ Extraction des volumes captés...${RESET}"

        grep "Spring #" "$INPUT" \
            | awk -F';' '{print $3 ";" $4}' \
            | sort -r >> "$OUTFILE"

        echo -e "${GREEN}✔️  Fichier créé :${RESET} $OUTFILE"
        echo -e "${BLUE}⏳ Temps : $(( $(date +%s) - start ))s${RESET}"
        echo -e "${BLUE}────────────────────────────────────────────${RESET}"
        exit 0
    fi

    ########################################################
    #                    HISTO REAL                        #
    ########################################################
    if [ "$MODE" = "real" ]; then
        echo -e "${BLUE}┌──────────────────────────────────────────┐${RESET}"
        echo -e "${BLUE}│   ${GREEN}Histogramme : Mode REAL${BLUE}                     │${RESET}"
        echo -e "${BLUE}└──────────────────────────────────────────┘${RESET}"

        OUTFILE="histo_real.dat"
        echo "identifier;real_volume(k.m3.year-1)" > "$OUTFILE"

        echo -e "${YELLOW}➤ Préparation des volumes réels...${RESET}"

        grep "Spring #" "$INPUT" \
            | awk -F';' '{print $3 ";" $4 ";" $5}' \
            | sort -r >> "$OUTFILE"

        echo -e "${GREEN}✔️  Fichier créé :${RESET} $OUTFILE"
        echo -e "${BLUE}⏳ Temps : $(( $(date +%s) - start ))s${RESET}"
        echo -e "${BLUE}────────────────────────────────────────────${RESET}"
        exit 0
    fi
fi

########################################################
#                COMMANDE INCONNUE                     #
########################################################
echo -e "${RED}✖️ Commande invalide : '$CMD'${RESET}"
exit 1
