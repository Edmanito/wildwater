#ifndef AVL_H
#define AVL_H

#include <stdio.h>

//Usine
typedef struct {
    char *id;
    double max;
    double src;
    double real;
} usine_t;

//AVL
typedef struct noeud_avl noeud_avl_t;

noeud_avl_t *avl_inserer(noeud_avl_t *racine, usine_t *usine);
usine_t    *avl_trouver(noeud_avl_t *racine, const char *id);
void        avl_ecrire(noeud_avl_t *racine, FILE *flux, int mode);
void        avl_liberer(noeud_avl_t *racine);

#endif