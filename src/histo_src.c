#define _POSIX_C_SOURCE 200809L  // pour getline()

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "histo_src.h"

typedef struct {
    char *id;      // identifiant usine (alloué dynamiquement)
    double somme;  // somme des volumes
} Usine;


static char *suppr_espaces(char *s) {
    if (!s) return s;

    // Supprime les espaces / tabulations / retours au début
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') s++;

    // la meme mais à la fin
    size_t n = strlen(s);
    while (n > 0) {
        char c = s[n - 1];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            s[n - 1] = '\0';
            n--;
        } else {
            break;
        }
    }
    return s;
}


static int trouver_usine(Usine *tab, int nb, const char *id) {
    for (int i = 0; i < nb; i++) {
        if (strcmp(tab[i].id, id) == 0) return i;
    }
    return -1;
}

/* Parse une ligne "id;volume"
   - met id_out (pointeur dans la ligne) et vol_out
   - retourne 1 si OK, 0 sinon */
static int parser_ligne(char *ligne, char **id_out, double *vol_out) {
    char *sep = strchr(ligne, ';');
    if (!sep) return 0;

    *sep = '\0';
    char *id = trim(ligne);
    char *voltxt = trim(sep + 1);

    if (id[0] == '\0') return 0;
    if (voltxt[0] == '\0') return 0;
    if (strcmp(voltxt, "-") == 0) return 0;

    // conversion double simple + vérif minimale
    char *fin = NULL;
    double v = strtod(voltxt, &fin);
    if (fin == voltxt) return 0;  // rien converti

    // accepter espaces en fin
    while (*fin == ' ' || *fin == '\t' || *fin == '\r' || *fin == '\n') fin++;
    if (*fin != '\0') return 0;   // caractères non numériques derrière

    *id_out = id;
    *vol_out = v;
    return 1;
}

int calculer_histo_src(const char *chemin_entree, const char *chemin_sortie) {
    FILE *f = fopen(chemin_entree, "r");
    if (!f) {
        perror("histo_src: fopen entree");
        return 1;
    }

    int capacite = 256;
    int nb = 0;
    Usine *usines = malloc(sizeof(Usine) * capacite);
    if (!usines) {
        fclose(f);
        fprintf(stderr, "histo_src: malloc impossible\n");
        return 2;
    }

    // lecture sans taille max
    char *ligne = NULL;
    size_t taille = 0;

    while (getline(&ligne, &taille, f) != -1) {
        char *l = trim(ligne);
        if (l[0] == '\0') continue;

        char *id = NULL;
        double volume = 0.0;

        if (!parser_ligne(l, &id, &volume)) {
            continue;
        }

        int idx = trouver_usine(usines, nb, id);
        if (idx >= 0) {
            usines[idx].somme += volume;
        } else {
            if (nb >= capacite) {
                capacite *= 2;
                Usine *tmp = realloc(usines, sizeof(Usine) * capacite);
                if (!tmp) {
                    fclose(f);
                    free(ligne);
                    // libérer les ids déjà alloués
                    for (int i = 0; i < nb; i++) free(usines[i].id);
                    free(usines);
                    fprintf(stderr, "histo_src: realloc impossible\n");
                    return 3;
                }
                usines = tmp;
            }

            usines[nb].id = malloc(strlen(id) + 1);
            if (!usines[nb].id) {
                fclose(f);
                free(ligne);
                for (int i = 0; i < nb; i++) free(usines[i].id);
                free(usines);
                fprintf(stderr, "histo_src: malloc id impossible\n");
                return 4;
            }
            strcpy(usines[nb].id, id);
            usines[nb].somme = volume;
            nb++;
        }
    }

    fclose(f);
    free(ligne);

    FILE *out = fopen(chemin_sortie, "w");
    if (!out) {
        perror("histo_src: fopen sortie");
        for (int i = 0; i < nb; i++) free(usines[i].id);
        free(usines);
        return 5;
    }

    // écriture non triée (tri fait côté shell)
    for (int i = 0; i < nb; i++) {
        fprintf(out, "%s;%.10g\n", usines[i].id, usines[i].somme);
    }

    fclose(out);

    for (int i = 0; i < nb; i++) free(usines[i].id);
    free(usines);

    return 0;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <entree_tmp> <sortie_tmp>\n", argv[0]);
        return 1;
    }
    return calculer_histo_src(argv[1], argv[2]);
}