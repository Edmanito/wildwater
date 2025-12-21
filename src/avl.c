#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avl.h"

struct noeud_avl {
    usine_t *u;
    int h;
    struct noeud_avl *g;
    struct noeud_avl *d;
};

static int h(noeud_avl_t *n) {
    return n ? n->h : 0;
}

static int max2(int a, int b) {
    return (a > b) ? a : b;
}

static int bal(noeud_avl_t *n) {
    return n ? (h(n->g) - h(n->d)) : 0;
}

static noeud_avl_t *rot_droite(noeud_avl_t *y) {
    noeud_avl_t *x = y->g;
    noeud_avl_t *t2 = x->d;
    x->d = y;
    y->g = t2;
    y->h = 1 + max2(h(y->g), h(y->d));
    x->h = 1 + max2(h(x->g), h(x->d));
    return x;
}

static noeud_avl_t *rot_gauche(noeud_avl_t *x) {
    noeud_avl_t *y = x->d;
    noeud_avl_t *t2 = y->g;
    y->g = x;
    x->d = t2;
    x->h = 1 + max2(h(x->g), h(x->d));
    y->h = 1 + max2(h(y->g), h(y->d));
    return y;
}

static noeud_avl_t *noeud_creer(usine_t *u) {
    noeud_avl_t *n = malloc(sizeof(*n));
    if (!n) return NULL;
    n->u = u;
    n->h = 1;
    n->g = NULL;
    n->d = NULL;
    return n;
}

noeud_avl_t *avl_inserer(noeud_avl_t *r, usine_t *u) {
    if (!u || !u->id) return r;

    if (!r) return noeud_creer(u);

    int c = strcmp(u->id, r->u->id);
    if (c < 0) {
        r->g = avl_inserer(r->g, u);
    } else if (c > 0) {
        r->d = avl_inserer(r->d, u);
    } else {
        return r;
    }

    r->h = 1 + max2(h(r->g), h(r->d));
    int b = bal(r);

    if (b > 1 && strcmp(u->id, r->g->u->id) < 0) return rot_droite(r);
    if (b < -1 && strcmp(u->id, r->d->u->id) > 0) return rot_gauche(r);
    if (b > 1 && strcmp(u->id, r->g->u->id) > 0) { r->g = rot_gauche(r->g); return rot_droite(r); }
    if (b < -1 && strcmp(u->id, r->d->u->id) < 0) { r->d = rot_droite(r->d); return rot_gauche(r); }

    return r;
}

usine_t *avl_trouver(noeud_avl_t *r, const char *id) {
    while (r) {
        int c = strcmp(id, r->u->id);
        if (c == 0) return r->u;
        r = (c < 0) ? r->g : r->d;
    }
    return NULL;
}

static double valeur_selon_mode(const usine_t *u, int mode) {
    if (!u) return 0.0;
    if (mode == 0) return u->max;
    if (mode == 1) return u->src;
    if (mode == 2) return u->real;
    return 0.0;
}

static void ecrire_infixe(noeud_avl_t *r, FILE *out, int mode) {
    if (!r) return;
    ecrire_infixe(r->g, out, mode);
    fprintf(out, "%s;%.10g\n", r->u->id, valeur_selon_mode(r->u, mode));
    ecrire_infixe(r->d, out, mode);
}

void avl_ecrire(noeud_avl_t *r, FILE *out, int mode) {
    if (!out) return;
    ecrire_infixe(r, out, mode);
}

static void liberer_usine(usine_t *u) {
    if (!u) return;
    free(u->id);
    free(u);
}

void avl_liberer(noeud_avl_t *r) {
    if (!r) return;
    avl_liberer(r->g);
    avl_liberer(r->d);
    liberer_usine(r->u);
    free(r);
}
