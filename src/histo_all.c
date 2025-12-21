#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "histo_all.h"

//Nettoie fin de ligne
static void enlever_retour_ligne(char *chaine) {
    if (!chaine) return;
    chaine[strcspn(chaine, "\r\n")] = '\0';
}

//Lire ligne id et valeur
static int lire_ligne_val(FILE *flux, char **tampon, size_t *taille,
                          char *id_sortie, size_t taille_id, double *valeur_sortie)
{
    while (1) {
        if (getline(tampon, taille, flux) == -1) return 0;

        enlever_retour_ligne(*tampon);

        if ((*tampon)[0] == '\0') continue;
        if (strncmp(*tampon, "identifier;", 11) == 0) continue;

        char *separateur = strchr(*tampon, ';');
        if (!separateur) continue;

        *separateur = '\0';
        char *ptr_id = *tampon;
        char *ptr_val = separateur + 1;

        strncpy(id_sortie, ptr_id, taille_id - 1);
        id_sortie[taille_id - 1] = '\0';

        char *fin_ptr = NULL;
        double x = strtod(ptr_val, &fin_ptr);
        if (fin_ptr == ptr_val) continue;

        *valeur_sortie = x;
        return 1;
    }
}

//Histo
int generer_histo_all(const char *chemin_max,
                      const char *chemin_src,
                      const char *chemin_real,
                      const char *chemin_sortie)
{
    FILE *flux_max  = fopen(chemin_max, "r");
    FILE *flux_src  = fopen(chemin_src, "r");
    FILE *flux_real = fopen(chemin_real, "r");
    FILE *flux_sortie  = fopen(chemin_sortie, "w");

    if (!flux_max || !flux_src || !flux_real || !flux_sortie) {
        if (flux_max) fclose(flux_max);
        if (flux_src) fclose(flux_src);
        if (flux_real) fclose(flux_real);
        if (flux_sortie) fclose(flux_sortie);
        fprintf(stderr, "Erreur ouverture fichiers\n");
        return 2;
    }

    fprintf(flux_sortie, "identifier;green;red;blue\n");

    char *tampon_src = NULL, *tampon_real = NULL, *tampon_max = NULL;
    size_t taille_src = 0, taille_real = 0, taille_max = 0;

    char id_src[1024]  = {0};
    char id_real[1024] = {0};
    double val_src = 0.0, val_real = 0.0;

    int res_src  = lire_ligne_val(flux_src,  &tampon_src,  &taille_src,  id_src,  sizeof(id_src),  &val_src);
    int res_real = lire_ligne_val(flux_real, &tampon_real, &taille_real, id_real, sizeof(id_real), &val_real);

    char id_max[1024];
    double val_max = 0.0;

    while (lire_ligne_val(flux_max, &tampon_max, &taille_max, id_max, sizeof(id_max), &val_max)) {

        while (res_src && strcmp(id_src, id_max) > 0) {
            res_src = lire_ligne_val(flux_src, &tampon_src, &taille_src, id_src, sizeof(id_src), &val_src);
        }
        while (res_real && strcmp(id_real, id_max) > 0) {
            res_real = lire_ligne_val(flux_real, &tampon_real, &taille_real, id_real, sizeof(id_real), &val_real);
        }

        double vol_src  = (res_src  && strcmp(id_src,  id_max) == 0) ? val_src  : 0.0;
        double vol_real = (res_real && strcmp(id_real, id_max) == 0) ? val_real : 0.0;

        double vert = val_max - vol_src;
        double rouge   = vol_src - vol_real;
        double bleu  = vol_real;

        if (vert < 0.0) vert = 0.0;
        if (rouge   < 0.0) rouge   = 0.0;
        if (bleu  < 0.0) bleu  = 0.0;

        fprintf(flux_sortie, "%s;%.10g;%.10g;%.10g\n", id_max, vert, rouge, bleu);
    }

    free(tampon_src);
    free(tampon_real);
    free(tampon_max);

    fclose(flux_max);
    fclose(flux_src);
    fclose(flux_real);
    fclose(flux_sortie);

    return 0;
}