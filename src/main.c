#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "histo_src.h"
#include "histo_real.h"
#include "histo_all.h"
#include "leaks.h"

//Fonction
static void usage(const char *prog) {
    fprintf(stderr,
        "Usage:\n"
        "  Histogrammes:\n"
        "    %s histo src  <entree_tmp> <sortie_tmp>\n"
        "    %s histo real <entree_tmp> <sortie_tmp>\n"
        "    %s histo all  <histo_max.csv> <histo_src.csv> <histo_real.csv> <histo_all.csv>\n"
        "\n"
        "  Leaks (mode fichiers tmp):\n"
        "    %s leaks <ID_USINE> <sources_tmp> <edges_tmp> <out_tmp>\n"
        "    (sources_tmp : ID;VOLUME_k;POURCENTAGE)\n"
        "    (edges_tmp   : AMONT;AVAL;POURCENTAGE_FUITE)\n"
        "    (out_tmp     : écrit une valeur unique en M.m3/an, ou -1)\n"
        "\n",
        prog, prog, prog, prog
    );
}

int main(int argc, char **argv) {
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    // HISTOGRAMMES
    // ===================

    if (strcmp(argv[1], "histo") == 0) {
        if (argc < 3) {
            usage(argv[0]);
            return 1;
        }

        //histo src
        if (strcmp(argv[2], "src") == 0) {
            if (argc != 5) { usage(argv[0]); return 1; }
            return generer_histo_src(argv[3], argv[4]);
        }

        //histo real
        if (strcmp(argv[2], "real") == 0) {
            if (argc != 5) { usage(argv[0]); return 1; }
            return calculer_histo_real(argv[3], argv[4]);
        }

        //histo all
        if (strcmp(argv[2], "all") == 0) {
            if (argc != 7) { usage(argv[0]); return 1; }
            return generer_histo_all(argv[3], argv[4], argv[5], argv[6]);
        }

        usage(argv[0]);
        return 2;
    }

    // LEAKS 
    // =========================
    if (strcmp(argv[1], "leaks") == 0) {
        
        if (argc != 6) {
            fprintf(stderr, "Erreur: leaks attend 4 arguments.\n");
            usage(argv[0]);
            return 1;
        }

        return calculer_leaks_usine(argv[2], argv[3], argv[4], argv[5]);
    }

    usage(argv[0]);
    return 2;
}
