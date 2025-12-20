#ifndef AVL_H
#define AVL_H

#include <stdio.h>

/* Usine */
typedef struct {
    char *id;
    double max;
    double src;
    double real;
} usine_t;

/* AVL opaque */
typedef struct noeud_avl noeud_avl_t;

/* Strict minimum */
noeud_avl_t *avl_inserer(noeud_avl_t *r, usine_t *u);
usine_t    *avl_trouver(noeud_avl_t *r, const char *id);
void        avl_ecrire(noeud_avl_t *r, FILE *out, int mode);
void        avl_liberer(noeud_avl_t *r);

#endif
