#!/bin/bash

export LC_ALL=C

#couleur
R='\033[0;31m' # Rouge (Erreur)
G='\033[0;32m' # Vert (Succès)
B='\033[0;34m' # Bleu (Info)
Y='\033[1;33m' # Jaune (Titre)
N='\033[0m'    # Neutre

if [ ! -f "exec/Wildwater" ]; then
    echo -e "${R}Erreur : L'exécutable exec/Wildwater est introuvable.${N}"
    echo -e "${R}Veuillez lancer 'make' avant d'exécuter ce script.${N}"
    exit 1
fi

START=$(date +%s%3N)

# Timer en miliseconde partout 
finish_ms() {
    code="$1"; shift
    [ $# -gt 0 ] && echo -e "${R}$*${N}"
    END=$(date +%s%3N)
    echo -e "${B}[INFO] Temps de traitement : $(( END - START )) ms${N}"
    exit "$code"
}

# On verifie les arguments
if [ $# -lt 3 ]; then
    echo -e "${Y}Usage :${N}"
    echo "  $0 data.csv histo {max|src|real|all}"
    echo "  $0 data.csv leaks <ID_USINE>"
    finish_ms 1
fi

FILE="$1"
ACT="$2" 
OPT="$3" 

if [ ! -f "$FILE" ]; then
    finish_ms 1 "Erreur : fichier $FILE introuvable"
fi


mkdir -p data data/histo data/dat data/tmp data/leaks


csv_to_dat() {
    IN="$1"
    OUT="$2"
    tail -n +2 "$IN" > "$OUT"
    echo -e "${G}[DAT] $OUT généré${N}"
}


case "$ACT" in histo)

#HISTOGRAMMES 


    
    # ---HISTO REAL ---# 
    
    if [ "$OPT" = "max" ]; then
        CSV="data/histo/histo_max.csv"
        DAT="data/dat/histo_max.dat"

        P_SM="data/histo/histo_max_50plus_petites.png"
        P_BG="data/histo/histo_max_10plus_grandes.png"

        echo -e "${Y}Histogramme MAX${N}"
        echo "identifier;max_volume(k.m3.year-1)" > "$CSV"

        grep '^-' "$FILE" \
        | grep ';-;' \
        | awk -F';' '$1=="-" && $3=="-" && $5=="-" && $4!="-" { print $2 ";" $4 }' \
        | sort -t';' -k1,1r \
        >> "$CSV"

        csv_to_dat "$CSV" "$DAT"

        D_SM="data/dat/histo_max_small50.dat"
        D_BG="data/dat/histo_max_big10.dat"

        sort -t';' -k2,2n  "$DAT" | awk -F';' 'NR<=50 { print $1 ";" $2 }' > "$D_SM"
        sort -t';' -k2,2nr "$DAT" | awk -F';' 'NR<=10 { print $1 ";" $2 }' > "$D_BG"

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

set output "${P_SM}"
set title "MAX — 50 plus petites usines (capacité maximale)"
set xlabel "Usines" offset 0, 6
set ylabel "Capacité maximale (M.m3/an)"
plot sprintf("< sort -t';' -k2,2n %s | awk 'NR<=50'", DATFILE) \
     using (\$2/1000.0):xticlabels(1) with boxes lc rgb "#3498db"

set output "${P_BG}"
set title "MAX — 10 plus grandes usines (capacité maximale)"
set xlabel "Usines" offset 0, 3
set ylabel "Capacité maximale (M.m3/an)"
plot sprintf("< sort -t';' -k2,2nr %s | awk 'NR<=10'", DATFILE) \
     using (\$2/1000.0):xticlabels(1) with boxes lc rgb "#3498db"
EOF

        [ $? -eq 0 ] || finish_ms 1 "[ERREUR] gnuplot a échoué"
        echo -e "${G}[INFO] PNG généré : $P_SM${N}"
        echo -e "${G}[INFO] PNG généré : $P_BG${N}"
        finish_ms 0
    fi

    
    # ---HISTO SRC ---# 
    
    if [ "$OPT" = "src" ]; then
        CSV="data/histo/histo_src.csv"
        DAT="data/dat/histo_src.dat"

        REF_SM="data/dat/histo_max_small50.dat"
        REF_BG="data/dat/histo_max_big10.dat"

        T_DIR="data/tmp"
        mkdir -p "$T_DIR"

        T_IN="${T_DIR}/src_input.tmp"
        T_CAL="${T_DIR}/src_calc.tmp"

        ID_SM="${T_DIR}/small50.ids"
        ID_BG="${T_DIR}/big10.ids"
        D_SM="${T_DIR}/histo_src_small50.dat"
        D_BG="${T_DIR}/histo_src_big10.dat"

        P_SM="data/histo/histo_src_50plus_petites.png"
        P_BG="data/histo/histo_src_10plus_grandes.png"

        BIN="exec/Wildwater"

        echo -e "${Y}Histogramme SRC${N}"

        if [ ! -f "$REF_SM" ] || [ ! -f "$REF_BG" ]; then
            echo -e "${Y}[WARN] Fichiers MAX manquants → génération MAX${N}"
            bash "$0" "$FILE" histo max
            [ -f "$REF_SM" ] && [ -f "$REF_BG" ] || finish_ms 1 "[ERREUR] MAX introuvable après génération."
        fi

        if [ ! -x "$BIN" ]; then
            echo -e "${B}[INFO] Compilation Wildwater...${N}"
            make || finish_ms 1 "[ERREUR] Compilation Wildwater impossible"
        fi

        echo "identifier;src_volume(k.m3.year-1)" > "$CSV"

        # entrée pour le C
        awk -F';' '$1=="-" && $3!="-" && $4!="-" && $5!="-" { print $3 ";" $4 }' \
            "$FILE" > "$T_IN"
        [ -s "$T_IN" ] || finish_ms 1 "[ERREUR] T_IN vide : aucun flux source->usine."

        "$BIN" histo src "$T_IN" "$T_CAL" \
            || finish_ms 1 "[ERREUR] histo src (C via main) a échoué"
        [ -s "$T_CAL" ] || finish_ms 1 "[ERREUR] Sortie C SRC vide ($T_CAL)."

        sort -t';' -k1,1r "$T_CAL" >> "$CSV"
        csv_to_dat "$CSV" "$DAT"

        awk -F';' '{print $1}' "$REF_SM" > "$ID_SM"
        awk -F';' '{print $1}' "$REF_BG" > "$ID_BG"

        awk -F';' 'FNR==NR { src[$1]=$2; next } { id=$0; print id ";" (id in src ? src[id] : 0) }' \
            "$DAT" "$ID_SM" > "$D_SM"
        awk -F';' 'FNR==NR { src[$1]=$2; next } { id=$0; print id ";" (id in src ? src[id] : 0) }' \
            "$DAT" "$ID_BG" > "$D_BG"

        [ -s "$D_SM" ] && [ -s "$D_BG" ] || finish_ms 1 "[ERREUR] Datasets SRC vides."

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

set output "${P_SM}"
set title "SRC — 50 plus petites usines"
set xlabel "Usines" offset 0, 3
set ylabel "Volume capté (M.m3/an)"
plot "${D_SM}" using 0:(column(2)/1000.0):xticlabels(1) with boxes lc rgb "#e74c3c"

set output "${P_BG}"
set title "SRC — 10 plus grandes usines"
set xlabel "Usines"
set ylabel "Volume capté (M.m3/an)"
plot "${D_BG}" using 0:(column(2)/1000.0):xticlabels(1) with boxes lc rgb "#e74c3c"
EOF

        [ $? -eq 0 ] || finish_ms 1 "[ERREUR] gnuplot SRC a échoué"
        echo -e "${G}[INFO] PNG généré : $P_SM${N}"
        echo -e "${G}[INFO] PNG généré : $P_BG${N}"
        finish_ms 0
    fi

    # ---HISTO REAL--- # 
    
    if [ "$OPT" = "real" ]; then
        CSV="data/histo/histo_real.csv"
        DAT="data/dat/histo_real.dat"

        REF_SM="data/dat/histo_max_small50.dat"
        REF_BG="data/dat/histo_max_big10.dat"

        T_DIR="data/tmp"
        mkdir -p "$T_DIR"

        T_IN="${T_DIR}/real_input.tmp"
        T_CAL="${T_DIR}/real_calc.tmp"

        ID_SM="${T_DIR}/real_small50.ids"
        ID_BG="${T_DIR}/real_big10.ids"
        D_SM="${T_DIR}/histo_real_small50.dat"
        D_BG="${T_DIR}/histo_real_big10.dat"

        P_SM="data/histo/histo_real_50plus_petites.png"
        P_BG="data/histo/histo_real_10plus_grandes.png"

        BIN="exec/Wildwater"

        echo -e "${Y}Histogramme REAL${N}"

        if [ ! -f "$REF_SM" ] || [ ! -f "$REF_BG" ]; then
            echo -e "${Y}[WARN] Fichiers MAX manquants → génération MAX${N}"
            bash "$0" "$FILE" histo max
            [ -f "$REF_SM" ] && [ -f "$REF_BG" ] || finish_ms 1 "[ERREUR] MAX introuvable après génération."
        fi

        if [ ! -x "$BIN" ]; then
            echo -e "${B}[INFO] Compilation Wildwater...${N}"
            make || finish_ms 1 "[ERREUR] Compilation Wildwater impossible"
        fi

        echo "identifier;real_volume(k.m3.year-1)" > "$CSV"

        # entrée : ID;   VOLUME    ;FUITE
        awk -F';' '$1=="-" && $3!="-" && $4!="-" && $5!="-" { print $3 ";" $4 ";" $5 }' \
            "$FILE" > "$T_IN"
        [ -s "$T_IN" ] || finish_ms 1 "[ERREUR] T_IN vide : aucun flux source->usine."

        "$BIN" histo real "$T_IN" "$T_CAL" \
            || finish_ms 1 "[ERREUR] histo real (C via main) a échoué"
        [ -s "$T_CAL" ] || finish_ms 1 "[ERREUR] Sortie C REAL vide ($T_CAL)."

        sort -t';' -k1,1r "$T_CAL" >> "$CSV"
        csv_to_dat "$CSV" "$DAT"

        awk -F';' '{print $1}' "$REF_SM" > "$ID_SM"
        awk -F';' '{print $1}' "$REF_BG" > "$ID_BG"

        awk -F';' 'FNR==NR { real[$1]=$2; next } { id=$0; print id ";" (id in real ? real[id] : 0) }' \
            "$DAT" "$ID_SM" > "$D_SM"
        awk -F';' 'FNR==NR { real[$1]=$2; next } { id=$0; print id ";" (id in real ? real[id] : 0) }' \
            "$DAT" "$ID_BG" > "$D_BG"

        [ -s "$D_SM" ] && [ -s "$D_BG" ] || finish_ms 1 "[ERREUR] Datasets REAL vides."

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

set output "${P_SM}"
set title "REAL — 50 plus petites usines"
set xlabel "Usines" offset 0, 6
set ylabel "Volume traité (M.m3/an)"
plot "${D_SM}" using 0:(column(2)/1000.0):xticlabels(1) with boxes lc rgb "#2ecc71"

set output "${P_BG}"
set title "REAL — 10 plus grandes usines"
set xlabel "Usines"
set ylabel "Volume traité (M.m3/an)"
plot "${D_BG}" using 0:(column(2)/1000.0):xticlabels(1) with boxes lc rgb "#2ecc71"
EOF

        [ $? -eq 0 ] || finish_ms 1 "[ERREUR] gnuplot REAL a échoué"
        echo -e "${G}[INFO] PNG généré : $P_SM${N}"
        echo -e "${G}[INFO] PNG généré : $P_BG${N}"
        finish_ms 0
    fi

    
    #--- HISTO ALL ---# 
    
    if [ "$OPT" = "all" ]; then
        C_OUT="data/histo/histo_all.csv"
        D_OUT="data/dat/histo_all.dat"

        C_MAX="data/histo/histo_max.csv"
        C_SRC="data/histo/histo_src.csv"
        C_REA="data/histo/histo_real.csv"

        REF_SM="data/dat/histo_max_small50.dat"
        REF_BG="data/dat/histo_max_big10.dat"

        T_DIR="data/tmp"
        mkdir -p "$T_DIR"

        ID_SM="${T_DIR}/all_small50.ids"
        ID_BG="${T_DIR}/all_big10.ids"
        D_SM="${T_DIR}/histo_all_small50.dat"
        D_BG="${T_DIR}/histo_all_big10.dat"

        P_SM="data/histo/histo_all_50plus_petites.png"
        P_BG="data/histo/histo_all_10plus_grandes.png"

        BIN="exec/Wildwater"

        echo -e "${Y}Histogramme ALL (bonus)${N}"
        echo -e "${B}[INFO] Re-génération complète (MAX, SRC, REAL)${N}"

        bash "$0" "$FILE" histo max  || finish_ms 1 "[ERREUR] histo max a échoué"
        bash "$0" "$FILE" histo src  || finish_ms 1 "[ERREUR] histo src a échoué"
        bash "$0" "$FILE" histo real || finish_ms 1 "[ERREUR] histo real a échoué"

        [ -f "$C_MAX" ] && [ -f "$C_SRC" ] && [ -f "$C_REA" ] || finish_ms 1 "[ERREUR] Fichiers max/src/real introuvables."
        [ -f "$REF_SM" ] && [ -f "$REF_BG" ] || finish_ms 1 "[ERREUR] Fichiers de référence MAX introuvables."

        if [ ! -x "$BIN" ]; then
            echo -e "${B}[INFO] Compilation Wildwater...${N}"
            make || finish_ms 1 "[ERREUR] Compilation Wildwater impossible"
        fi

        "$BIN" histo all "$C_MAX" "$C_SRC" "$C_REA" "$C_OUT" \
            || finish_ms 1 "[ERREUR] histo_all (C via main) a échoué"

        csv_to_dat "$C_OUT" "$D_OUT"

        awk -F';' '{print $1}' "$REF_SM" > "$ID_SM"
        awk -F';' '{print $1}' "$REF_BG" > "$ID_BG"

        awk -F';' 'FNR==NR { v[$1]=$2";"$3";"$4; next } { id=$0; if (id in v) print id";"v[id]; else print id";0;0;0" }' \
            "$D_OUT" "$ID_SM" > "$D_SM"
        awk -F';' 'FNR==NR { v[$1]=$2";"$3";"$4; next } { id=$0; if (id in v) print id";"v[id]; else print id";0;0;0" }' \
            "$D_OUT" "$ID_BG" > "$D_BG"

        [ -s "$D_SM" ] && [ -s "$D_BG" ] || finish_ms 1 "[ERREUR] Datasets ALL vides."

        gnuplot <<EOF
set terminal pngcairo size 1800,900 enhanced font "Arial,10"
set datafile separator ";"
unset key
set grid ytics
set yrange [0:*]

set style data histograms
set style histogram rowstacked
set style fill solid 0.85 border -1
set boxwidth 0.75

set style line 1 lc rgb "#3498db"
set style line 2 lc rgb "#e74c3c"
set style line 3 lc rgb "#2ecc71"

set xtics rotate by -90 center offset 0,-10
set tmargin 2
set bmargin 22

set output "${P_SM}"
set title "ALL — 50 plus petites usines"
set xlabel "Usines" offset 0, 6
set ylabel "Volumes (M.m3/an)"
plot "${D_SM}" using (column(4)/1000.0):xticlabels(1) ls 1, \
     "" using (column(3)/1000.0) ls 2, \
     "" using (column(2)/1000.0) ls 3

set output "${P_BG}"
set title "ALL — 10 plus grandes usines"
set xlabel "Usines"
set ylabel "Volumes (M.m3/an)"
plot "${D_BG}" using (column(4)/1000.0):xticlabels(1) ls 1, \
     "" using (column(3)/1000.0) ls 2, \
     "" using (column(2)/1000.0) ls 3

unset output
EOF

        [ $? -eq 0 ] || finish_ms 1 "[ERREUR] gnuplot ALL a échoué"
        echo -e "${G}[INFO] PNG généré : $P_SM${N}"
        echo -e "${G}[INFO] PNG généré : $P_BG${N}"
        finish_ms 0
    fi

    finish_ms 1 "Erreur : mode histo inconnu"
    ;;

# ==== LEAKS ==== #

leaks)
    
    TARGET="$OPT"


    # Sécurité : ID usine valide
    # =========================
    if ! awk -F';' -v ID="$TARGET" '
        $2==ID || $3==ID || $1==ID { found=1 }
        END { exit(found ? 0 : 1) }
    ' "$FILE"
    then
        finish_ms 1 "Erreur : '$TARGET' n'est pas un identifiant d'usine valide"
    fi


    T_DIR="data/tmp"
    mkdir -p "$T_DIR"

    T_SRC="${T_DIR}/leaks_sources.tmp"
    T_EDG="${T_DIR}/leaks_edges.tmp"
    T_OUT="${T_DIR}/leaks_out.tmp"

    L_DIR="data/leaks"
    mkdir -p "$L_DIR"
    L_DAT="${L_DIR}/leaks.dat"

    BIN="exec/Wildwater"

    echo -e "${Y}LEAKS pour : $TARGET${N}"

    if [ ! -x "$BIN" ]; then
        echo -e "${B}[INFO] Compilation Wildwater...${N}"
        make || finish_ms 1 "[ERREUR] Compilation impossible"
    fi

   
    awk -F';' -v ID="$TARGET" '
        $1=="-" && $3==ID && $4!="-" && $5!="-" { print $3 ";" $4 ";" $5 }
    ' "$FILE" > "$T_SRC"

  
    awk -F';' -v ID="$TARGET" '
        # USINE -> STOCKAGE (col1 "-", col2 usine, col3 stockage, col4 "-", col5 fuite)
        $1=="-" && $2==ID && $3!="-" && $4=="-" && $5!="-" {
            print $2 ";" $3 ";" $5;
            next
        }

        # Tout le réseau aval (col1 = usine)
        $1==ID && $2!="-" && $3!="-" && $5!="-" {
            print $2 ";" $3 ";" $5
        }
    ' "$FILE" > "$T_EDG"

    #Sécurité
    : > "$T_OUT"

    "$BIN" leaks "$TARGET" "$T_SRC" "$T_EDG" "$T_OUT" \
        || finish_ms 1 "[ERREUR] leaks (C) a échoué"

    [ -s "$T_OUT" ] || finish_ms 1 "[ERREUR] sortie leaks vide ($T_OUT)"

    VAL="$(tr -d '\r\n' < "$T_OUT")"

    echo "$VAL" | grep -Eq '^-?[0-9]+([.][0-9]+)?$' \
        || finish_ms 1 "[ERREUR] sortie leaks invalide: '$VAL'"

    if [ ! -f "$L_DAT" ]; then
        echo "identifier;Leak volume (M.m3.year-1)" > "$L_DAT"
    fi
    echo "$TARGET;$VAL" >> "$L_DAT"

    echo -e "${G}[INFO] Leak volume = $VAL M.m3/year${N}"
    echo -e "${B}[INFO] Historique : $L_DAT${N}"
    finish_ms 0
    ;;




# == COMMANDE INCONNUE == #
*)
    finish_ms 1 "Erreur : commande inconnue"
    ;;
esac