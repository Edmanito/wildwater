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

static int hauteur(noeud_avl_t *noeud) {
    return noeud ? noeud->hauteur : 0;
}

static int max(int a, int b) {
    return (a > b) ? a : b;
}

static int equilibre(noeud_avl_t *noeud) {
    return noeud ? (hauteur(noeud->gauche) - hauteur(noeud->droite)) : 0;
}

static noeud_avl_t *rotation_droite(noeud_avl_t *y) {
    noeud_avl_t *x = y->gauche;
    noeud_avl_t *t2 = x->droite;
    
    x->droite = y;
    y->gauche = t2;
    
    y->hauteur = 1 + max(hauteur(y->gauche), hauteur(y->droite));
    x->hauteur = 1 + max(hauteur(x->gauche), hauteur(x->droite));
    
    return x;
}

static noeud_avl_t *rotation_gauche(noeud_avl_t *x) {
    noeud_avl_t *y = x->droite;
    noeud_avl_t *t2 = y->gauche;
    
    y->gauche = x;
    x->droite = t2;
    
    x->hauteur = 1 + max(hauteur(x->gauche), hauteur(x->droite));
    y->hauteur = 1 + max(hauteur(y->gauche), hauteur(y->droite));
    
    return y;
}

static noeud_avl_t *noeud_creer(usine_t *usine) {
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

    if (!racine) return noeud_creer(usine);

    int comparaison = strcmp(usine->id, racine->usine->id);
    
    if (comparaison < 0) {
        racine->gauche = avl_inserer(racine->gauche, usine);
    } else if (comparaison > 0) {
        racine->droite = avl_inserer(racine->droite, usine);
    } else {
        return racine;
    }

    racine->hauteur = 1 + max(hauteur(racine->gauche), hauteur(racine->droite));
    
    int facteur = equilibre(racine);

    if (facteur > 1 && strcmp(usine->id, racine->gauche->usine->id) < 0) 
        return rotation_droite(racine);
    
    if (facteur < -1 && strcmp(usine->id, racine->droite->usine->id) > 0) 
        return rotation_gauche(racine);
    
    if (facteur > 1 && strcmp(usine->id, racine->gauche->usine->id) > 0) {
        racine->gauche = rotation_gauche(racine->gauche);
        return rotation_droite(racine);
    }
    
    if (facteur < -1 && strcmp(usine->id, racine->droite->usine->id) < 0) {
        racine->droite = rotation_droite(racine->droite);
        return rotation_gauche(racine);
    }

    return racine;
}

usine_t *avl_trouver(noeud_avl_t *racine, const char *id) {
    while (racine) {
        int comparaison = strcmp(id, racine->usine->id);
        if (comparaison == 0) return racine->usine;
        racine = (comparaison < 0) ? racine->gauche : racine->droite;
    }
    return NULL;
}

static double valeur_selon_mode(const usine_t *usine, int mode) {
    if (!usine) return 0.0;
    if (mode == 0) return usine->max;
    if (mode == 1) return usine->src;
    if (mode == 2) return usine->real;
    return 0.0;
}

static void ecrire_infixe(noeud_avl_t *racine, FILE *flux, int mode) {
    if (!racine) return;
    
    ecrire_infixe(racine->gauche, flux, mode);
    fprintf(flux, "%s;%.10g\n", racine->usine->id, valeur_selon_mode(racine->usine, mode));
    ecrire_infixe(racine->droite, flux, mode);
}

void avl_ecrire(noeud_avl_t *racine, FILE *flux, int mode) {
    if (!flux) return;
    ecrire_infixe(racine, flux, mode);
}

static void liberer_usine(usine_t *usine) {
    if (!usine) return;
    free(usine->id);
    free(usine);
}

void avl_liberer(noeud_avl_t *racine) {
    if (!racine) return;
    
    avl_liberer(racine->gauche);
    avl_liberer(racine->droite);
    
    liberer_usine(racine->usine);
    free(racine);
}