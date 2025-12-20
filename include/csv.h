#ifndef CSV_H
#define CSV_H

#include <stdio.h>

/* Ligne CSV minimale */
typedef struct {
    char *c1;
    char *c2;
    char *c3;
    double c4;
    double c5;
} ligne_csv_t;

/* Lecture */
int  lire_ligne_csv(FILE *f, ligne_csv_t *l);
void liberer_ligne_csv(ligne_csv_t *l);

/* Tests indispensables */
int est_usine(ligne_csv_t *l);
int est_source_vers_usine(ligne_csv_t *l);
int est_troncon(ligne_csv_t *l);

#endif
