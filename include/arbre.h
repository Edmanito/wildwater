#ifndef ARBRE_H
#define ARBRE_H

typedef struct noeud noeud_t;

/* Minimum vital */
noeud_t *obtenir_noeud(const char *id);
void     ajouter_arete(const char *amont,
                       const char *aval,
                       double fuite);
double   calculer_fuites(noeud_t *racine, double debit);
void     liberer_graphe(void);

#endif
