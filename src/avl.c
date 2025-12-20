#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------------- STRUCTURE AVL ---------------- */

typedef struct NoeudAVL {
    char *id;
    double valeur;
    int hauteur;
    struct NoeudAVL *gauche;
    struct NoeudAVL *droite;
} NoeudAVL;

/* -------------- FONCTIONS SIMPLES -------------- */

int hauteur(NoeudAVL *n) {
    if (n == NULL)
        return 0;
    return n->hauteur;
}

int maximum(int a, int b) {
    if (a > b)
        return a;
    else
        return b;
}

/* ---------------- ROTATIONS AVL ---------------- */

NoeudAVL *rotation_droite(NoeudAVL *y) {
    NoeudAVL *x = y->gauche;
    NoeudAVL *T2 = x->droite;

    x->droite = y;
    y->gauche = T2;

    y->hauteur = maximum(hauteur(y->gauche), hauteur(y->droite)) + 1;
    x->hauteur = maximum(hauteur(x->gauche), hauteur(x->droite)) + 1;

    return x;
}

NoeudAVL *rotation_gauche(NoeudAVL *x) {
    NoeudAVL *y = x->droite;
    NoeudAVL *T2 = y->gauche;

    y->gauche = x;
    x->droite = T2;

    x->hauteur = maximum(hauteur(x->gauche), hauteur(x->droite)) + 1;
    y->hauteur = maximum(hauteur(y->gauche), hauteur(y->droite)) + 1;

    return y;
}

int facteur_equilibre(NoeudAVL *n) {
    if (n == NULL)
        return 0;
    return hauteur(n->gauche) - hauteur(n->droite);
}

/* -------------- CREATION DE NOEUD --------------- */


NoeudAVL *creer_noeud(char *id, double valeur) {
    NoeudAVL *n = malloc(sizeof(NoeudAVL));

    n->id = malloc(strlen(id) + 1);
    strcpy(n->id, id);

    n->valeur = valeur;
    n->gauche = NULL;
    n->droite = NULL;
    n->hauteur = 1;

    return n;
}

