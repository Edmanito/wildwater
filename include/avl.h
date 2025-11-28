#ifndef AVL_H
#define AVL_H

// Structure d’un nœud d’AVL
typedef struct{
    char id[64];            // Identifiant de la station
    double valeur1;         // Première valeur
    double valeur2;         // Deuxième valeur
    double valeur3;         // Troisième valeur
    int hauteur;            // Hauteur du nœud
    struct Avl *fg;         // Fils gauche
    struct Avl *fd;         // Fils droit
} AVL;

// Fonctions AVL
AVL* insererAVL(AVL *racine, const char *id, double v1, double v2, double v3);
void afficherAVL(const AVL *racine);
int hauteurAVL(const AVL *noeud);

#endif
