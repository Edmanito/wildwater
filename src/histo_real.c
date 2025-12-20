#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "histo_real.h"

typedef struct {
    char id[256];
    double somme_real;
} Usine;

/* Supprime espaces/tab/retours début+fin, renvoie un pointeur dans la chaîne */
static char *suppr_espaces(char *s) {
    if (!s) return s;

    while (*s && isspace((unsigned char)*s)) s++;

    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) {
        s[n - 1] = '\0';
        n--;
    }
    return s;
}

static int trouver_usine(Usine *tab, int nb, const char *id) {
    for (int i = 0; i < nb; i++) {
        if (strcmp(tab[i].id, id) == 0) return i;
    }
    return -1;
}

/* Convertit un texte en double, renvoie 1 si OK sinon 0 */
static int lire_double(const char *txt, double *out) {
    if (!txt || txt[0] == '\0') return 0;
    if (strcmp(txt, "-") == 0) return 0;

    char *fin = NULL;
    double v = strtod(txt, &fin);

    if (fin == txt) return 0; // rien lu

    while (*fin && isspace((unsigned char)*fin)) fin++;
    if (*fin != '\0') return 0; // texte “sale” derrière

    *out = v;
    return 1;
}

/*
 * Parse une ligne "ID;VOLUME;FUITE"
 * Remplit id_out, volume_out, fuite_out si OK.
 */
static int parser_ligne(char *ligne, char *id_out, size_t taille_id,
                        double *volume_out, double *fuite_out) {
    char *p1 = strchr(ligne, ';');
    if (!p1) return 0;
    *p1 = '\0';

    char *reste = p1 + 1;
    char *p2 = strchr(reste, ';');
    if (!p2) return 0;
    *p2 = '\0';

    char *id = suppr_espaces(ligne);
    char *txt_vol = suppr_espaces(reste);
    char *txt_fuite = suppr_espaces(p2 + 1);

    if (id[0] == '\0') return 0;

    double volume = 0.0, fuite = 0.0;
    if (!lire_double(txt_vol, &volume)) return 0;
    if (!lire_double(txt_fuite, &fuite)) return 0;

    /* borne la fuite entre 0 et 100 (sécurité simple) */
    if (fuite < 0.0) fuite = 0.0;
    if (fuite > 100.0) fuite = 100.0;

    strncpy(id_out, id, taille_id - 1);
    id_out[taille_id - 1] = '\0';

    *volume_out = volume;
    *fuite_out = fuite;
    return 1;
}

int calculer_histo_real(const char *chemin_entree, const char *chemin_sortie) {
    FILE *f = fopen(chemin_entree, "r");
    if (!f) {
        perror("histo_real: fopen entree");
        return 1;
    }

    int capacite = 1024;
    int nb = 0;
    Usine *usines = malloc(sizeof(Usine) * capacite);
    if (!usines) {
        fclose(f);
        fprintf(stderr, "histo_real: malloc impossible\n");
        return 2;
    }

    char *ligne = NULL;
    size_t taille = 0;

    while (getline(&ligne, &taille, f) != -1) {
        char *l = suppr_espaces(ligne);
        if (l[0] == '\0') continue;

        char id[256];
        double volume = 0.0;
        double fuite = 0.0;

        if (!parser_ligne(l, id, sizeof(id), &volume, &fuite)) {
            continue;
        }

        double volume_reel = volume * (1.0 - fuite / 100.0);

        int idx = trouver_usine(usines, nb, id);
        if (idx >= 0) {
            usines[idx].somme_real += volume_reel;
        } else {
            if (nb >= capacite) {
                capacite *= 2;
                Usine *tmp = realloc(usines, sizeof(Usine) * capacite);
                if (!tmp) {
                    free(ligne);
                    fclose(f);
                    free(usines);
                    fprintf(stderr, "histo_real: realloc impossible\n");
                    return 3;
                }
                usines = tmp;
            }

            strncpy(usines[nb].id, id, sizeof(usines[nb].id) - 1);
            usines[nb].id[sizeof(usines[nb].id) - 1] = '\0';
            usines[nb].somme_real = volume_reel;
            nb++;
        }
    }

    free(ligne);
    fclose(f);

    FILE *out = fopen(chemin_sortie, "w");
    if (!out) {
        perror("histo_real: fopen sortie");
        free(usines);
        return 4;
    }

    /* écriture non triée (tri côté shell) */
    for (int i = 0; i < nb; i++) {
        fprintf(out, "%s;%.10g\n", usines[i].id, usines[i].somme_real);
    }

    fclose(out);
    free(usines);
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <entree_tmp> <sortie_tmp>\n", argv[0]);
        return 1;
    }
    return calculer_histo_real(argv[1], argv[2]);
}