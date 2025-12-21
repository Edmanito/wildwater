#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avl.h"

struct noeud_avl {
    usine_t *usine;
    int hauteur;
    struct noeud_avl *gauche;
    struct noeud_avl *droite;
};

static int hauteurnoeud(noeud_avl_t *noeud) {
    return noeud ? noeud->hauteur : 0;
}

static int maximum(int a, int b) {
    return (a > b) ? a : b;
}

static int equilibre(noeud_avl_t *noeud) {
    return noeud ? (hauteurnoeud(noeud->gauche) - hauteurnoeud(noeud->droite)) : 0;
}

static noeud_avl_t *rotationdroite(noeud_avl_t *racine) {
    noeud_avl_t *nouvelRacine = racine->gauche;
    noeud_avl_t *sousarbre = nouvelRacine->droite;

    nouvelRacine->droite = racine;
    racine->gauche = sousarbre;

    racine->hauteur = 1 + maximum(hauteurnoeud(racine->gauche), hauteurnoeud(racine->droite));
    nouvelRacine->hauteur = 1 + maximum(hauteurnoeud(nouvelRacine->gauche), hauteurnoeud(nouvelRacine->droite));

    return nouvelRacine;
}

static noeud_avl_t *rotationgauche(noeud_avl_t *racine) {
    noeud_avl_t *nouvelRacine = racine->droite;
    noeud_avl_t *sousarbre = nouvelRacine->gauche;

    nouvelRacine->gauche = racine;
    racine->droite = sousarbre;

    racine->hauteur = 1 + maximum(hauteurnoeud(racine->gauche), hauteurnoeud(racine->droite));
    nouvelRacine->hauteur = 1 + maximum(hauteurnoeud(nouvelRacine->gauche), hauteurnoeud(nouvelRacine->droite));

    return nouvelRacine;
}

static noeud_avl_t *creernoeud(usine_t *usine) {
    noeud_avl_t *noeud = malloc(sizeof(*noeud));
    if (!noeud) return NULL;

    noeud->usine = usine;
    noeud->hauteur = 1;
    noeud->gauche = NULL;
    noeud->droite = NULL;

    return noeud;
}

noeud_avl_t *avl_inserer(noeud_avl_t *racine, usine_t *usine) {
    if (!usine || !usine->id) return racine;

    if (!racine)
        return creernoeud(usine);

    int comparaison = strcmp(usine->id, racine->usine->id);

    if (comparaison < 0)
        racine->gauche = avl_inserer(racine->gauche, usine);
    else if (comparaison > 0)
        racine->droite = avl_inserer(racine->droite, usine);
    else
        return racine;

    racine->hauteur = 1 + maximum(hauteurnoeud(racine->gauche), hauteurnoeud(racine->droite));
    int facteur = equilibre(racine);

    if (facteur > 1 && strcmp(usine->id, racine->gauche->usine->id) < 0)
        return rotationdroite(racine);

    if (facteur < -1 && strcmp(usine->id, racine->droite->usine->id) > 0)
        return rotationgauche(racine);

    if (facteur > 1 && strcmp(usine->id, racine->gauche->usine->id) > 0) {
        racine->gauche = rotationgauche(racine->gauche);
        return rotationdroite(racine);
    }

    if (facteur < -1 && strcmp(usine->id, racine->droite->usine->id) < 0) {
        racine->droite = rotationdroite(racine->droite);
        return rotationgauche(racine);
    }

    return racine;
}

usine_t *avl_trouver(noeud_avl_t *racine, const char *id) {
    while (racine) {
        int comparaison = strcmp(id, racine->usine->id);
        if (comparaison == 0)
            return racine->usine;

        racine = (comparaison < 0) ? racine->gauche : racine->droite;
    }
    return NULL;
}

static double valeurmode(const usine_t *usine, int mode) {
    if (!usine) return 0.0;
    if (mode == 0) return usine->max;
    if (mode == 1) return usine->src;
    if (mode == 2) return usine->real;
    return 0.0;
}

static void ecrireinfixe(noeud_avl_t *racine, FILE *sortie, int mode) {
    if (!racine) return;

    ecrireinfixe(racine->gauche, sortie, mode);
    fprintf(sortie, "%s;%.10g\n", racine->usine->id, valeurmode(racine->usine, mode));
    ecrireinfixe(racine->droite, sortie, mode);
}

void avl_ecrire(noeud_avl_t *racine, FILE *sortie, int mode) {
    if (!sortie) return;
    ecrireinfixe(racine, sortie, mode);
}

static void libererusine(usine_t *usine) {
    if (!usine) return;
    free(usine->id);
    free(usine);
}

void avl_liberer(noeud_avl_t *racine) {
    if (!racine) return;

    avl_liberer(racine->gauche);
    avl_liberer(racine->droite);

    libererusine(racine->usine);
    free(racine);
}
