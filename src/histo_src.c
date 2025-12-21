#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "histo_src.h"
#include "avl.h"

static char *trim(char *s) {
    if (!s) return s;
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') s++;
    size_t n = strlen(s);
    while (n > 0) {
        char c = s[n - 1];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            s[n - 1] = '\0';
            n--;
        } else break;
    }
    return s;
}

static usine_t *usine_nouvelle(const char *id) {
    usine_t *u = malloc(sizeof(*u));
    if (!u) return NULL;
    u->id = malloc(strlen(id) + 1);
    if (!u->id) { free(u); return NULL; }
    strcpy(u->id, id);
    u->max = 0.0;
    u->src = 0.0;
    u->real = 0.0;
    return u;
}

static int parser_ligne(char *ligne, char **id_out, double *vol_out) {
    if (!ligne) return 0;
    if (strncmp(ligne, "identifier;", 11) == 0) return 0;

    char *sep = strchr(ligne, ';');
    if (!sep) return 0;

    *sep = '\0';
    char *id = trim(ligne);
    char *voltxt = trim(sep + 1);

    if (id[0] == '\0') return 0;
    if (voltxt[0] == '\0') return 0;
    if (strcmp(voltxt, "-") == 0) return 0;

    char *fin = NULL;
    double v = strtod(voltxt, &fin);
    if (fin == voltxt) return 0;

    while (*fin == ' ' || *fin == '\t' || *fin == '\r' || *fin == '\n') fin++;
    if (*fin != '\0') return 0;

    *id_out = id;
    *vol_out = v;
    return 1;
}

int generer_histo_src(const char *chemin_entree, const char *chemin_sortie) {
    FILE *f = fopen(chemin_entree, "r");
    if (!f) { perror("histo_src: fopen entree"); return 1; }

    noeud_avl_t *racine = NULL;
    char *ligne = NULL;
    size_t taille = 0;

    while (getline(&ligne, &taille, f) != -1) {
        char *l = trim(ligne);
        if (l[0] == '\0') continue;

        char *id = NULL;
        double volume = 0.0;
        if (!parser_ligne(l, &id, &volume)) continue;

        usine_t *u = avl_trouver(racine, id);
        if (!u) {
            u = usine_nouvelle(id);
            if (!u) { free(ligne); fclose(f); avl_liberer(racine); return 2; }
            racine = avl_inserer(racine, u);
            if (!racine) { free(ligne); fclose(f); free(u->id); free(u); return 2; }
            u = avl_trouver(racine, id);
        }
        if (u) u->src += volume;
    }

    free(ligne);
    fclose(f);

    FILE *out = fopen(chemin_sortie, "w");
    if (!out) { perror("histo_src: fopen sortie"); avl_liberer(racine); return 5; }

    avl_ecrire(racine, out, 1);
    fclose(out);

    avl_liberer(racine);
    return 0;
}
