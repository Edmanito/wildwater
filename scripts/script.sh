#!/bin/bash

start=$(date +%s)

# Vérification des arguments
if [ $# -lt 2 ]; then
    echo "Usage : $0 <fichier.csv> histo {max|src|real} | leaks <ID>"
    exit 1
fi

INPUT="$1"
CMD="$2"
MODE="$3"

if [ ! -f "$INPUT" ]; then
    echo "Erreur : fichier introuvable : $INPUT"
    exit 1
fi

mkdir -p data

##############################################
#                    HISTO                  #
##############################################

if [ "$CMD" = "histo" ]; then

    if [ -z "$MODE" ]; then
        echo "Erreur : mode requis (max | src | real)"
        exit 1
    fi

    if [ "$MODE" != "max" ] && [ "$MODE" != "src" ] && [ "$MODE" != "real" ]; then
        echo "Erreur : mode inconnu '$MODE'"
        exit 1
    fi

    ##############################################
    #                 HISTO MAX                  #
    ##############################################
    if [ "$MODE" = "max" ]; then
    echo "Histogramme (mode max)"
    OUTFILE="data/histo_max.dat"
    mkdir -p data

    echo "max_volume(k.m3.year-1)" > "$OUTFILE"

    echo "Extraction en cours..."
    awk -F';' '
        $1 == "-" && $3 == "-" && $5 == "-" {
            print $4
        }
    ' "$INPUT" | sort -nr >> "$OUTFILE"

    echo "Fichier généré : $OUTFILE"
    echo "Terminé en $(( $(date +%s) - start )) secondes"
    exit 0
fi


#mettre le ^ juste apres le grep
    ##############################################
    #                 HISTO SRC                  #
    ##############################################
    if [ "$MODE" = "src" ]; then
        echo "Histogramme (mode src)"
        OUTFILE="data/histo_src.dat"

        echo "identifier;source_volume(k.m3.year-1)" > "$OUTFILE"

        echo "Extraction en cours..."
        grep "Spring #" "$INPUT" \
            | awk -F';' '{print $3 ";" $4}' \
            | sort -r >> "$OUTFILE"

        echo "Fichier généré : $OUTFILE"
        echo "Terminé en $(( $(date +%s) - start )) secondes"
        exit 0
    fi

    ##############################################
    #                 HISTO REAL                 #
    ##############################################
    if [ "$MODE" = "real" ]; then
        echo "Histogramme (mode real)"
        OUTFILE="data/histo_real.dat"

        echo "identifier;real_volume(k.m3.year-1)" > "$OUTFILE"

        echo "Extraction en cours..."
        grep "Spring #" "$INPUT" \
            | awk -F';' '{print $3 ";" $4 ";" $5}' \
            | sort -r >> "$OUTFILE"

        echo "Fichier généré : $OUTFILE"
        echo "Terminé en $(( $(date +%s) - start )) secondes"
        exit 0
    fi
fi

echo "Erreur : commande inconnue '$CMD'"
exit 1


