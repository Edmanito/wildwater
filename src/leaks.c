#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leaks.h"

/*
 * Implémentation simple et robuste du calcul des fuites.
 * Toute la logique est ici.
 * Aucun main dans ce fichier.
 *
 * Hypothèse simplifiée :
 * - on calcule le volume réellement traité à l'usine
 * - on applique les pourcentages de fuite de tous les tronçons aval
 *   traités par cette usine
 */

int leaks_compute(
    const char *csv_path,
    const char *plant_id,
    const char *out_csv,
    const char *history_dat
) {
    FILE *f = fopen(csv_path, "r");
    if (!f) return 1;

    char line[2048];
    int plant_found = 0;
    double treated_volume = 0.0;
    double total_leaks = 0.0;

    /* ============================================================
     * PASS 1 : calcul du volume réellement traité par l'usine
     * (SOURCE -> USINE)
     * ============================================================ */
    while (fgets(line, sizeof(line), f)) {
        char *c1 = strtok(line, ";");      // col 1
        char *c2 = strtok(NULL, ";");      // col 2
        char *c3 = strtok(NULL, ";");      // col 3
        char *c4 = strtok(NULL, ";");      // volume
        char *c5 = strtok(NULL, ";\n");    // fuite %

        if (!c1 || !c2 || !c3 || !c4 || !c5)
            continue;

        /* SOURCE -> USINE */
        if (strcmp(c1, "-") == 0 && strcmp(c3, plant_id) == 0) {
            double vol = atof(c4);
            double leak = atof(c5);
            treated_volume += vol * (1.0 - leak / 100.0);
            plant_found = 1;
        }

        /* Ligne descriptive de l'usine */
        if (strcmp(c1, "-") == 0 && strcmp(c2, plant_id) == 0) {
            plant_found = 1;
        }
    }

    fclose(f);

    /* ============================================================
     * Usine inexistante
     * ============================================================ */
    if (!plant_found) {
        FILE *o = fopen(out_csv, "w");
        if (!o) return 2;
        fprintf(o, "identifier;Leak volume (M.m3.year-1)\n");
        fprintf(o, "%s;-1\n", plant_id);
        fclose(o);

        FILE *h = fopen(history_dat, "a");
        if (h) {
            fprintf(h, "%s;-1\n", plant_id);
            fclose(h);
        }

        return 0;
    }

    /* ===== PASS 2 : fuites directes usine ===== */
f = fopen(csv_path, "r");
if (!f) return 2;

int out_count = 0;

/* --- Comptage des tronçons sortants --- */
while (fgets(line, sizeof(line), f)) {
    char *c1 = strtok(line, ";");
    strtok(NULL, ";");
    strtok(NULL, ";");
    strtok(NULL, ";");
    char *c5 = strtok(NULL, ";\n");

    if (!c1 || !c5) continue;

    if (strcmp(c1, plant_id) == 0) {
        out_count++;
    }
}

if (out_count == 0) out_count = 1;

/* Revenir au début du fichier */
rewind(f);

/* --- Calcul des fuites réparties --- */
while (fgets(line, sizeof(line), f)) {
    char *c1 = strtok(line, ";");
    strtok(NULL, ";");
    strtok(NULL, ";");
    strtok(NULL, ";");
    char *c5 = strtok(NULL, ";\n");

    if (!c1 || !c5) continue;

    if (strcmp(c1, plant_id) == 0) {
        double leak = atof(c5);
        double share = treated_volume / out_count;
        total_leaks += share * (leak / 100.0);
    }
}

fclose(f);


    /* Conversion en millions de m³ */
    double leaks_Mm3 = total_leaks / 1000.0;

    /* ============================================================
     * Sorties
     * ============================================================ */
    FILE *o = fopen(out_csv, "w");
    if (!o) return 4;
    fprintf(o, "identifier;Leak volume (M.m3.year-1)\n");
    fprintf(o, "%s;%.6f\n", plant_id, leaks_Mm3);
    fclose(o);

    FILE *h = fopen(history_dat, "a");
    if (h) {
        fprintf(h, "%s;%.6f\n", plant_id, leaks_Mm3);
        fclose(h);
    }

    return 0;
}
