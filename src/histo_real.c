#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "histo_real.h"
#include "avl.h"

static int est_espace(char c) {
    return (c == ' '  || c == '\t' || c == '\n' ||
            c == '\r' || c == '\v' || c == '\f');
}

static char *suppr_espaces(char *s) {
    if (!s) return s;
    while (*s && est_espace(*s)) s++;
    size_t n = strlen(s);
    while (n > 0 && est_espace(s[n - 1])) { s[n - 1] = '\0'; n--; }
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

static int lire_double(const char *txt, double *out) {
    if (!txt || txt[0] == '\0') return 0;
    if (strcmp(txt, "-") == 0) return 0;

    char *fin = NULL;
    double v = strtod(txt, &fin);
    if (fin == txt) return 0;

    while (*fin && est_espace(*fin)) fin++;
    if (*fin != '\0') return 0;

    *out = v;
    return 1;
}

static int parser_ligne(char *ligne, char *id_out, size_t taille_id,
                        double *volume_out, double *fuite_out)
{
    if (!ligne) return 0;
    if (strncmp(ligne, "identifier;", 11) == 0) return 0;

    char *p1 = strchr(ligne, ';');
    if (!p1) return 0;
    *p1 = '\0';

    char *reste = p1 + 1;
    char *p2 = strchr(reste, ';');
    if (!p2) return 0;
    *p2 = '\0';

    char *id = suppr_espaces(ligne);
    char *txt_vol = suppr_espaces(reste);
    char *txt_fuite = suppr_espaces(p2 + 1);

    if (id[0] == '\0') return 0;

    double volume = 0.0, fuite = 0.0;
    if (!lire_double(txt_vol, &volume)) return 0;
    if (!lire_double(txt_fuite, &fuite)) return 0;

    if (fuite < 0.0) fuite = 0.0;
    if (fuite > 100.0) fuite = 100.0;

    strncpy(id_out, id, taille_id - 1);
    id_out[taille_id - 1] = '\0';

    *volume_out = volume;
    *fuite_out = fuite;
    return 1;
}

int calculer_histo_real(const char *chemin_entree, const char *chemin_sortie) {
    FILE *f = fopen(chemin_entree, "r");
    if (!f) { perror("histo_real: fopen entree"); return 1; }

    noeud_avl_t *racine = NULL;
    char *ligne = NULL;
    size_t taille = 0;

    while (getline(&ligne, &taille, f) != -1) {
        char *l = suppr_espaces(ligne);
        if (l[0] == '\0') continue;

        char id[256];
        double volume = 0.0;
        double fuite = 0.0;

        if (!parser_ligne(l, id, sizeof(id), &volume, &fuite)) continue;

        double volume_reel = volume * (1.0 - fuite / 100.0);

        usine_t *u = avl_trouver(racine, id);
        if (!u) {
            u = usine_nouvelle(id);
            if (!u) { free(ligne); fclose(f); avl_liberer(racine); return 2; }
            racine = avl_inserer(racine, u);
            if (!racine) { free(ligne); fclose(f); free(u->id); free(u); return 2; }
            u = avl_trouver(racine, id);
        }
        if (u) u->real += volume_reel;
    }

    free(ligne);
    fclose(f);

    FILE *out = fopen(chemin_sortie, "w");
    if (!out) { perror("histo_real: fopen sortie"); avl_liberer(racine); return 4; }

    avl_ecrire(racine, out, 2);
    fclose(out);

    avl_liberer(racine);
    return 0;
}
