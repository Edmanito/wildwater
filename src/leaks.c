#include "../include/arbre.h"
#include "../include/leaks.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static double total_fuites_global = 0.0;

static void propager_flux(Noeud* noeud, double volume_actuel) {
    if (!noeud || !noeud->liste_enfants) return;

    int nb = 0;
    for (Arete* a = noeud->liste_enfants; a; a = a->suivant) nb++;
    if (nb == 0) return;

    double volume_par_tuyau = volume_actuel / nb;

    for (Arete* a = noeud->liste_enfants; a; a = a->suivant) {
        double perte = volume_par_tuyau * (a->pourcentage_fuite / 100.0);
        total_fuites_global += perte;
        propager_flux(a->enfant, volume_par_tuyau - perte);
    }
}


static double calc_volume_traite_k(const char* sources_tmp, const char* id_usine) {
    FILE* f = fopen(sources_tmp, "r");
    if (!f) return 0.0;

    char ligne[512];
    char id[256];
    double V = 0.0, P = 0.0;
    double total = 0.0;

    while (fgets(ligne, sizeof(ligne), f)) {
        ligne[strcspn(ligne, "\r\n")] = 0;

        if (sscanf(ligne, " %255[^;];%lf;%lf", id, &V, &P) == 3) {
            if (strcmp(id, id_usine) == 0) {
                total += V * (1.0 - P / 100.0);
            }
        }
    }

    fclose(f);
    return total;
}

static int charger_edges(const char* edges_tmp) {
    FILE* f = fopen(edges_tmp, "r");
    if (!f) return 3;

    char ligne[512];
    char amont[256], aval[256];
    double P = 0.0;

    while (fgets(ligne, sizeof(ligne), f)) {
        ligne[strcspn(ligne, "\r\n")] = 0;

        if (sscanf(ligne, " %255[^;];%255[^;];%lf", amont, aval, &P) == 3) {
            ajouter_arete(amont, aval, P);
        }
    }

    fclose(f);
    return 0;
}

int calculer_leaks_usine(const char* id_usine,
                         const char* sources_tmp,
                         const char* edges_tmp,
                         const char* out_tmp) {
    if (!id_usine || !sources_tmp || !edges_tmp || !out_tmp) return 1;

    total_fuites_global = 0.0;

    int err = charger_edges(edges_tmp);
    if (err) {
        FILE* out = fopen(out_tmp, "w");
        if (out) { fprintf(out, "-1\n"); fclose(out); }
        liberer_tout();
        return err;
    }

    Noeud* depart = obtenir_noeud(id_usine);
    if (!depart) {
        FILE* out = fopen(out_tmp, "w");
        if (out) { fprintf(out, "-1\n"); fclose(out); }
        liberer_tout();
        return 2;
    }

    double vol_traite_k = calc_volume_traite_k(sources_tmp, id_usine);

    // Calcul des fuites 
    propager_flux(depart, vol_traite_k);


    FILE* out = fopen(out_tmp, "w");
    if (!out) {
        liberer_tout();
        return 4;
    }
    fprintf(out, "%.6f\n", total_fuites_global / 1000.0);
    fclose(out);

    liberer_tout();
    return 0;
}
