#!/bin/bash

export LC_ALL=C
START_TIME_MS=$(date +%s%3N)

# ====== Timer (ms) partout ======
finish_ms() {
    code="$1"; shift
    [ $# -gt 0 ] && echo "$*"
    END_TIME_MS=$(date +%s%3N)
    echo "[INFO] Temps de traitement : $(( END_TIME_MS - START_TIME_MS )) ms"
    exit "$code"
}

##############################################
# Vérification des arguments
##############################################

if [ $# -lt 3 ]; then
    echo "Usage :"
    echo "  $0 data.csv histo {max|src|real|all}"
    echo "  $0 data.csv leaks <ID_USINE>"
    finish_ms 1
fi

INPUT="$1"
CMD="$2"
MODE="$3"

if [ ! -f "$INPUT" ]; then
    finish_ms 1 "Erreur : fichier $INPUT introuvable"
fi

##############################################
# Création des dossiers
##############################################

mkdir -p data data/histo data/dat data/tmp data/leaks

##############################################
# Conversion CSV -> DAT (gnuplot)
##############################################
csv_to_dat() {
    IN="$1"
    OUT="$2"
    tail -n +2 "$IN" > "$OUT"
    echo "[DAT] $OUT généré"
}

##############################################
# ================= HISTOGRAMMES =============
##############################################
if [ "$CMD" = "histo" ]; then

##############################################
# HISTO MAX 
##############################################
if [ "$MODE" = "max" ]; then
    CSV="data/histo/histo_max.csv"
    DAT="data/dat/histo_max.dat"

    PNG_SMALL="data/histo/histo_max_50plus_petites.png"
    PNG_BIG="data/histo/histo_max_10plus_grandes.png"

    echo "Histogramme MAX"

    echo "identifier;max_volume(k.m3.year-1)" > "$CSV"

    # Filtrage selon consigne + tri par identifiant (ordre alphabétique inverse)
    grep '^-' "$INPUT" \
    | grep ';-;' \
    | awk -F';' '$1=="-" && $3=="-" && $5=="-" && $4!="-" { print $2 ";" $4 }' \
    | sort -t';' -k1,1r \
    >> "$CSV"

    # Conversion CSV -> DAT (sans en-tête)
    csv_to_dat "$CSV" "$DAT"

	SMALL_DAT="data/dat/histo_max_small50.dat"
	BIG_DAT="data/dat/histo_max_big10.dat"

	sort -t';' -k2,2n  "$DAT" | awk -F';' 'NR<=50 { print $1 ";" $2 }' > "$SMALL_DAT"
	sort -t';' -k2,2nr "$DAT" | awk -F';' 'NR<=10 { print $1 ";" $2 }' > "$BIG_DAT"


    # Génération des 2 graphiques (50 plus petites / 10 plus grandes)
gnuplot <<EOF
set datafile separator ";"
set style fill solid 0.85 border -1
set boxwidth 0.75
set grid ytics
set key off
set xtics rotate by -90 center offset 0,-8
set tmargin 2
set bmargin 16

set terminal pngcairo size 1800,900 enhanced font "Arial,10"

DATFILE="${DAT}"

# ---- 50 plus petites ----
set output "${PNG_SMALL}"
set title "MAX — 50 plus petites usines (capacité maximale)"
set xlabel "Usines" offset 0, 6
set ylabel "Capacité maximale (M.m3/an)"
plot sprintf("< sort -t';' -k2,2n %s | awk 'NR<=50'", DATFILE) \
     using (\$2/1000.0):xticlabels(1) with boxes

# ---- 10 plus grandes ----
set output "${PNG_BIG}"
set title "MAX — 10 plus grandes usines (capacité maximale)"
set xlabel "Usines" offset 0, 3
set ylabel "Capacité maximale (M.m3/an)"
plot sprintf("< sort -t';' -k2,2nr %s | awk 'NR<=10'", DATFILE) \
     using (\$2/1000.0):xticlabels(1) with boxes
EOF

    if [ $? -ne 0 ]; then
        finish_ms 1 "[ERREUR] gnuplot a échoué"
    fi

    echo "[INFO] PNG généré : $PNG_SMALL"
    echo "[INFO] PNG généré : $PNG_BIG"

    finish_ms 0
fi



##############################################
# HISTO SRC (volume total capté par usine)
##############################################
if [ "$MODE" = "src" ]; then
    CSV="data/histo/histo_src.csv"
    DAT="data/dat/histo_src.dat"

    # Référence MAX (doit exister)
    MAX_SMALL="data/dat/histo_max_small50.dat"
    MAX_BIG="data/dat/histo_max_big10.dat"

    TMP_DIR="data/tmp"
    mkdir -p "$TMP_DIR"

    SMALL_IDS="${TMP_DIR}/small50.ids"
    BIG_IDS="${TMP_DIR}/big10.ids"
    SMALL_DAT="${TMP_DIR}/histo_src_small50.dat"
    BIG_DAT="${TMP_DIR}/histo_src_big10.dat"

    PNG_SMALL="data/histo/histo_src_50plus_petites.png"
    PNG_BIG="data/histo/histo_src_10plus_grandes.png"

    echo "Histogramme SRC"

    # Vérif fichiers de référence MAX (auto-génération si manquants)
    if [ ! -f "$MAX_SMALL" ] || [ ! -f "$MAX_BIG" ]; then
        echo "[WARN] Fichiers MAX manquants ($MAX_SMALL / $MAX_BIG)."
        echo "[WARN] Lancement automatique : histo max"
        bash "$0" "$INPUT" histo max

        # Re-vérification après génération
        if [ ! -f "$MAX_SMALL" ] || [ ! -f "$MAX_BIG" ]; then
            finish_ms 1 "[ERREUR] Impossible de générer les fichiers MAX ($MAX_SMALL / $MAX_BIG)."
        fi
    fi

    echo "identifier;src_volume(k.m3.year-1)" > "$CSV"

    # Somme des volumes captés par usine (source -> usine)
    awk -F';' '
        $1=="-" && $3!="-" && $4!="-" && $5!="-" { print $3 ";" $4 }
    ' "$INPUT" \
    | sort -t';' -k1,1 \
    | awk -F';' '
        NR==1 { prev=$1; sum=$2; next }
        $1==prev { sum+=$2; next }
        { print prev ";" sum; prev=$1; sum=$2 }
        END { if (NR>0) print prev ";" sum }
    ' \
    | sort -t';' -k1,1r \
    >> "$CSV"

    # CSV -> DAT (sans en-tête)
    csv_to_dat "$CSV" "$DAT"

    # IDs des 50 petites / 10 grandes selon MAX (ordre déjà bon)
    awk -F';' '{print $1}' "$MAX_SMALL" > "$SMALL_IDS"
    awk -F';' '{print $1}' "$MAX_BIG"   > "$BIG_IDS"

    # Construire 2 datasets SRC dans l'ordre des IDs (réf MAX)
    awk -F';' '
        FNR==NR { src[$1]=$2; next }
        { id=$0; print id ";" (id in src ? src[id] : 0) }
    ' "$DAT" "$SMALL_IDS" > "$SMALL_DAT"

    awk -F';' '
        FNR==NR { src[$1]=$2; next }
        { id=$0; print id ";" (id in src ? src[id] : 0) }
    ' "$DAT" "$BIG_IDS" > "$BIG_DAT"

    # Sécurité : datasets non vides
    if [ ! -s "$SMALL_DAT" ] || [ ! -s "$BIG_DAT" ]; then
        finish_ms 1 "[ERREUR] Datasets SRC vides ($SMALL_DAT / $BIG_DAT)"
    fi

    # Génération des 2 graphiques (barres)
    gnuplot <<EOF
set terminal pngcairo size 1800,900 enhanced font "Arial,10"
set datafile separator ";"
set style fill solid 0.85 border -1
set boxwidth 0.75
set grid ytics
set key off
set yrange [0:*]

set xtics rotate by -90 center offset 0,-10
set tmargin 2
set bmargin 22

# ---- 50 plus petites (réf MAX) ----
set output "${PNG_SMALL}"
set title "SRC — 50 plus petites usines (réf. capacité MAX)"
set xlabel "Usines" offset 0, 3
set ylabel "Volume capté (M.m3/an)"
plot "${SMALL_DAT}" using 0:(column(2)/1000.0):xticlabels(1) with boxes

# ---- 10 plus grandes (réf MAX) ----
set output "${PNG_BIG}"
set title "SRC — 10 plus grandes usines (réf. capacité MAX)"
set xlabel "Usines"
set ylabel "Volume capté (M.m3/an)"
plot "${BIG_DAT}" using 0:(column(2)/1000.0):xticlabels(1) with boxes
EOF

    if [ $? -ne 0 ]; then
        finish_ms 1 "[ERREUR] gnuplot SRC a échoué"
    fi

    echo "[INFO] PNG généré : $PNG_SMALL"
    echo "[INFO] PNG généré : $PNG_BIG"

    finish_ms 0
fi



##############################################
# HISTO REAL (volume réellement traité) — réf MAX
##############################################
if [ "$MODE" = "real" ]; then
    CSV="data/histo/histo_real.csv"
    DAT="data/dat/histo_real.dat"

    # Référence MAX (capacité max = référence petites/grandes)
    MAX_SMALL="data/dat/histo_max_small50.dat"
    MAX_BIG="data/dat/histo_max_big10.dat"

    TMP_DIR="data/tmp"
    mkdir -p "$TMP_DIR"

    REAL_INPUT="${TMP_DIR}/real_input.tmp"
    REAL_CALC="${TMP_DIR}/real_calc.tmp"

    SMALL_IDS="${TMP_DIR}/real_small50.ids"
    BIG_IDS="${TMP_DIR}/real_big10.ids"
    SMALL_DAT="${TMP_DIR}/histo_real_small50.dat"
    BIG_DAT="${TMP_DIR}/histo_real_big10.dat"

    PNG_SMALL="data/histo/histo_real_50plus_petites.png"
    PNG_BIG="data/histo/histo_real_10plus_grandes.png"

    # Exécutable C (compilé via make)
    HISTO_REAL_EXEC="exec/histo_real"

    echo "Histogramme REAL"

    # Vérif fichiers de référence MAX (auto-génération si manquants)
    if [ ! -f "$MAX_SMALL" ] || [ ! -f "$MAX_BIG" ]; then
        echo "[WARN] Fichiers MAX manquants ($MAX_SMALL / $MAX_BIG)."
        echo "[WARN] Lancement automatique : histo max"
        bash "$0" "$INPUT" histo max

        if [ ! -f "$MAX_SMALL" ] || [ ! -f "$MAX_BIG" ]; then
            finish_ms 1 "[ERREUR] Impossible de générer les fichiers MAX ($MAX_SMALL / $MAX_BIG)."
        fi
    fi

    # Compilation via make si l'exécutable n'existe pas (pas d'appel direct à gcc)
    if [ ! -x "$HISTO_REAL_EXEC" ]; then
        echo "[INFO] Compilation histo_real..."
        make histo-real-bin || finish_ms 1 "[ERREUR] Compilation histo_real impossible"
    fi

    echo "identifier;real_volume(k.m3.year-1)" > "$CSV"

    # 1) Filtrage : on prépare l'entrée pour le C
    # Format attendu par le C : ID_USINE;VOLUME;FUITE_PERCENT
    awk -F';' '
        $1=="-" && $3!="-" && $4!="-" && $5!="-" {
            print $3 ";" $4 ";" $5
        }
    ' "$INPUT" > "$REAL_INPUT"

    if [ ! -s "$REAL_INPUT" ]; then
        finish_ms 1 "[ERREUR] Aucun flux source->usine valide pour REAL (fichier $REAL_INPUT vide)."
    fi

    # 2) Calcul en C : agrégation par usine -> output: ID_USINE;SOMME_REAL
    "$HISTO_REAL_EXEC" "$REAL_INPUT" "$REAL_CALC" || finish_ms 1 "[ERREUR] histo_real (C) a échoué"

    if [ ! -s "$REAL_CALC" ]; then
        finish_ms 1 "[ERREUR] Sortie C REAL vide ($REAL_CALC)."
    fi

    # 3) Tri complet (consigne) : par identifiant inverse
    sort -t';' -k1,1r "$REAL_CALC" >> "$CSV"

    # 4) CSV -> DAT (sans en-tête)
    csv_to_dat "$CSV" "$DAT"

    # 5) Construire les datasets dans l'ordre des IDs (réf MAX)
    awk -F';' '{print $1}' "$MAX_SMALL" > "$SMALL_IDS"
    awk -F';' '{print $1}' "$MAX_BIG"   > "$BIG_IDS"

    awk -F';' '
        FNR==NR { real[$1]=$2; next }
        { id=$0; print id ";" (id in real ? real[id] : 0) }
    ' "$DAT" "$SMALL_IDS" > "$SMALL_DAT"

    awk -F';' '
        FNR==NR { real[$1]=$2; next }
        { id=$0; print id ";" (id in real ? real[id] : 0) }
    ' "$DAT" "$BIG_IDS" > "$BIG_DAT"

    # Sécurité : datasets non vides
    if [ ! -s "$SMALL_DAT" ] || [ ! -s "$BIG_DAT" ]; then
        finish_ms 1 "[ERREUR] Datasets REAL vides ($SMALL_DAT / $BIG_DAT)"
    fi

    # 6) PNG (barres) — unité en M.m3/an => /1000
    gnuplot <<EOF
set terminal pngcairo size 1800,900 enhanced font "Arial,10"
set datafile separator ";"
set style fill solid 0.85 border -1
set boxwidth 0.75
set grid ytics
set key off
set yrange [0:*]

set xtics rotate by -90 center offset 0,-10
set tmargin 2
set bmargin 22

# ---- 50 plus petites (réf MAX) ----
set output "${PNG_SMALL}"
set title "REAL — 50 plus petites usines (réf. capacité MAX)"
set xlabel "Usines"
set ylabel "Volume traité (M.m3/an)"
plot "${SMALL_DAT}" using 0:(column(2)/1000.0):xticlabels(1) with boxes

# ---- 10 plus grandes (réf MAX) ----
set output "${PNG_BIG}"
set title "REAL — 10 plus grandes usines (réf. capacité MAX)"
set xlabel "Usines"
set ylabel "Volume traité (M.m3/an)"
plot "${BIG_DAT}" using 0:(column(2)/1000.0):xticlabels(1) with boxes
EOF

    if [ $? -ne 0 ]; then
        finish_ms 1 "[ERREUR] gnuplot REAL a échoué"
    fi

    echo "[INFO] PNG généré : $PNG_SMALL"
    echo "[INFO] PNG généré : $PNG_BIG"

    finish_ms 0
fi



##############################################
# HISTO ALL (BONUS) : histogramme cumulé
# Couleurs / ordre (bas -> haut) :
#   BLEU  = real
#   ROUGE = source - real
#   VERT  = max - source
##############################################
if [ "$MODE" = "all" ]; then
    CSV_OUT="data/histo/histo_all.csv"
    DAT_OUT="data/dat/histo_all.dat"

    # On s'appuie sur les sorties déjà générées :
    MAX_CSV="data/histo/histo_max.csv"
    SRC_CSV="data/histo/histo_src.csv"
    REAL_CSV="data/histo/histo_real.csv"

    # Référence (capacité max -> petites/grandes)
    MAX_SMALL="data/dat/histo_max_small50.dat"
    MAX_BIG="data/dat/histo_max_big10.dat"

    TMP_DIR="data/tmp"
    mkdir -p "$TMP_DIR"

    SMALL_IDS="${TMP_DIR}/all_small50.ids"
    BIG_IDS="${TMP_DIR}/all_big10.ids"
    SMALL_DAT="${TMP_DIR}/histo_all_small50.dat"
    BIG_DAT="${TMP_DIR}/histo_all_big10.dat"

    PNG_SMALL="data/histo/histo_all_50plus_petites.png"
    PNG_BIG="data/histo/histo_all_10plus_grandes.png"

    echo "Histogramme ALL (bonus)"
    echo "[INFO] Re-génération complète (MAX, SRC, REAL) pour garantir la cohérence avec : $INPUT"

    # IMPORTANT : on régénère TOUJOURS tout avec le même INPUT
    if ! bash "$0" "$INPUT" histo max; then
        finish_ms 1 "[ERREUR] histo max a échoué"
    fi
    if ! bash "$0" "$INPUT" histo src; then
        finish_ms 1 "[ERREUR] histo src a échoué"
    fi
    if ! bash "$0" "$INPUT" histo real; then
        finish_ms 1 "[ERREUR] histo real a échoué"
    fi

    # Vérif présence des fichiers attendus
    if [ ! -f "$MAX_CSV" ] || [ ! -f "$SRC_CSV" ] || [ ! -f "$REAL_CSV" ]; then
        finish_ms 1 "[ERREUR] Fichiers max/src/real introuvables après génération."
    fi
    if [ ! -f "$MAX_SMALL" ] || [ ! -f "$MAX_BIG" ]; then
        finish_ms 1 "[ERREUR] Fichiers de référence MAX introuvables ($MAX_SMALL / $MAX_BIG)."
    fi

    # Exécutable C histo_all
    HISTO_ALL_EXEC="exec/histo_all"
    if [ ! -x "$HISTO_ALL_EXEC" ]; then
        echo "[INFO] Compilation histo_all..."
        make histo-all-bin || finish_ms 1 "[ERREUR] compilation histo_all échouée"
    fi

    # Appel C : construit histo_all.csv
    # (ton C doit produire : id ; (max-src) ; (src-real) ; real)
    "$HISTO_ALL_EXEC" "$MAX_CSV" "$SRC_CSV" "$REAL_CSV" "$CSV_OUT" || finish_ms 1 "[ERREUR] histo_all (C) a échoué"

    # CSV -> DAT (sans en-tête)
    csv_to_dat "$CSV_OUT" "$DAT_OUT"

    # IDs des 50 petites / 10 grandes (réf MAX)
    awk -F';' '{print $1}' "$MAX_SMALL" > "$SMALL_IDS"
    awk -F';' '{print $1}' "$MAX_BIG"   > "$BIG_IDS"

    # Construire les 2 datasets ALL dans l'ordre des IDs (réf MAX)
    # DAT_OUT format : id;max-src;src-real;real
    awk -F';' '
        FNR==NR { v[$1]=$2";"$3";"$4; next }
        { id=$0; if (id in v) print id";"v[id]; else print id";0;0;0" }
    ' "$DAT_OUT" "$SMALL_IDS" > "$SMALL_DAT"

    awk -F';' '
        FNR==NR { v[$1]=$2";"$3";"$4; next }
        { id=$0; if (id in v) print id";"v[id]; else print id";0;0;0" }
    ' "$DAT_OUT" "$BIG_IDS" > "$BIG_DAT"

    # Sécurité : datasets non vides
    if [ ! -s "$SMALL_DAT" ] || [ ! -s "$BIG_DAT" ]; then
        finish_ms 1 "[ERREUR] Datasets ALL vides ($SMALL_DAT / $BIG_DAT)"
    fi

    # Gnuplot : histogramme empilé (ordre bas->haut = bleu, rouge, vert)
    gnuplot <<EOF
set terminal pngcairo size 1800,900 enhanced font "Arial,10"
set datafile separator ";"
set key top right
set grid ytics
set yrange [0:*]

set style data histograms
set style histogram rowstacked
set style fill solid 0.85 border -1
set boxwidth 0.75

# styles couleurs
set style line 1 lc rgb "#3498db"  # BLEU (real) -> bas
set style line 2 lc rgb "#e74c3c"  # ROUGE (src-real) -> milieu
set style line 3 lc rgb "#2ecc71"  # VERT (max-src) -> haut

set xtics rotate by -90 center offset 0,-10
set tmargin 2
set bmargin 22

# ---- 50 plus petites (réf MAX) ----
set output "${PNG_SMALL}"
set title "ALL — 50 plus petites usines (réf. capacité MAX)"
set xlabel "Usines"
set ylabel "Volumes (M.m3/an)"
plot "${SMALL_DAT}" using (column(4)/1000.0):xticlabels(1) title "real" ls 1, \
     "" using (column(3)/1000.0) title "source - real" ls 2, \
     "" using (column(2)/1000.0) title "max - source" ls 3

# ---- 10 plus grandes (réf MAX) ----
set output "${PNG_BIG}"
set title "ALL — 10 plus grandes usines (réf. capacité MAX)"
set xlabel "Usines"
set ylabel "Volumes (M.m3/an)"
plot "${BIG_DAT}" using (column(4)/1000.0):xticlabels(1) title "real" ls 1, \
     "" using (column(3)/1000.0) title "source - real" ls 2, \
     "" using (column(2)/1000.0) title "max - source" ls 3

unset output
EOF

    if [ $? -ne 0 ]; then
        finish_ms 1 "[ERREUR] gnuplot ALL a échoué"
    fi

    echo "[INFO] PNG généré : $PNG_SMALL"
    echo "[INFO] PNG généré : $PNG_BIG"
    finish_ms 0
fi


fi   # FIN du if CMD=histo

##############################################
# ================== LEAKS ===================
##############################################
if [ "$CMD" = "leaks" ]; then

    if [ $# -ne 3 ]; then
        finish_ms 1 "Usage : $0 data.csv leaks <ID_USINE>"
    fi

    PLANT_ID="$MODE"

    OUT_CSV="data/leaks/leaks.csv"
    HISTORY_DAT="data/leaks/leaks_history.dat"

    EXEC="./exec/Wildwater"

    if [ ! -x "$EXEC" ]; then
        echo "[INFO] Compilation Wildwater..."
        make || finish_ms 1 "[ERREUR] Compilation impossible"
    fi

    echo "[INFO] Calcul des fuites pour : $PLANT_ID"

    "$EXEC" leaks "$INPUT" "$PLANT_ID" "$OUT_CSV" "$HISTORY_DAT"
    RC=$?

    if [ $RC -ne 0 ]; then
        finish_ms $RC "[ERREUR] Échec du calcul des fuites"
    fi

    echo "[INFO] Résultat écrit dans $OUT_CSV"
    echo "[INFO] Historique mis à jour : $HISTORY_DAT"

    finish_ms 0
fi

##############################################
# Commande inconnue
##############################################
finish_ms 1 "Erreur : commande inconnue"
