
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "histo_all.h"

//supprime le \n et le \r 
static void enlever_retour_ligne(char *s) {
    if (!s) return;
    s[strcspn(s, "\r\n")] = '\0';
}

static int lire_ligne_val(FILE *f, char **ligne, size_t *cap,
                          char *id, size_t idsz, double *val)
{
    while (1) {
        if (getline(ligne, cap, f) == -1) return 0; 

        enlever_retour_ligne(*ligne);

        if ((*ligne)[0] == '\0') continue;
        if (strncmp(*ligne, "identifier;", 11) == 0) continue;

        char *sep = strchr(*ligne, ';');
        if (!sep) continue;

        *sep = '\0';
        char *id_txt = *ligne;
        char *val_txt = sep + 1;

        strncpy(id, id_txt, idsz - 1);
        id[idsz - 1] = '\0';

        char *fin = NULL;
        double x = strtod(val_txt, &fin);
        if (fin == val_txt) continue;

        *val = x;
        return 1;
    }
}


int generer_histo_all(const char *csv_max,
                      const char *csv_src,
                      const char *csv_real,
                      const char *csv_out)
{
    FILE *fmax  = fopen(csv_max, "r");
    FILE *fsrc  = fopen(csv_src, "r");
    FILE *freal = fopen(csv_real, "r");
    FILE *fout  = fopen(csv_out, "w");

    if (!fmax || !fsrc || !freal || !fout) {
        if (fmax) fclose(fmax);
        if (fsrc) fclose(fsrc);
        if (freal) fclose(freal);
        if (fout) fclose(fout);
        fprintf(stderr, "[ERREUR] Ouverture fichiers histo_all impossible\n");
        return 2;
    }

    fprintf(fout, "identifier;green;red;blue\n");

    char *ligne_src = NULL, *ligne_real = NULL, *ligne_max = NULL;
    size_t cap_src = 0, cap_real = 0, cap_max = 0;

    char id_src[1024]  = {0};
    char id_real[1024] = {0};
    double v_src = 0.0, v_real = 0.0;

    int a_src  = lire_ligne_val(fsrc,  &ligne_src,  &cap_src,  id_src,  sizeof(id_src),  &v_src);
    int a_real = lire_ligne_val(freal, &ligne_real, &cap_real, id_real, sizeof(id_real), &v_real);

    char id_max[1024];
    double v_max = 0.0;

    while (lire_ligne_val(fmax, &ligne_max, &cap_max, id_max, sizeof(id_max), &v_max)) {

        while (a_src && strcmp(id_src, id_max) > 0) {
            a_src = lire_ligne_val(fsrc, &ligne_src, &cap_src, id_src, sizeof(id_src), &v_src);
        }
        while (a_real && strcmp(id_real, id_max) > 0) {
            a_real = lire_ligne_val(freal, &ligne_real, &cap_real, id_real, sizeof(id_real), &v_real);
        }

        double src  = (a_src  && strcmp(id_src,  id_max) == 0) ? v_src  : 0.0;
        double real = (a_real && strcmp(id_real, id_max) == 0) ? v_real : 0.0;

        double green = v_max - src;
        double red   = src - real;
        double blue  = real;

        if (green < 0.0) green = 0.0;
        if (red   < 0.0) red   = 0.0;
        if (blue  < 0.0) blue  = 0.0;

        fprintf(fout, "%s;%.10g;%.10g;%.10g\n", id_max, green, red, blue);
    }

    free(ligne_src);
    free(ligne_real);
    free(ligne_max);

    fclose(fmax);
    fclose(fsrc);
    fclose(freal);
    fclose(fout);

    return 0;
}
