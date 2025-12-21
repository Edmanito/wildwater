#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "histo_src.h"
#include "histo_real.h"
#include "histo_all.h"
#include "leaks.h"


static void usage(const char *prog) {
    fprintf(stderr,
        "Usage:\n"
        "  Histogrammes:\n"
        "    %s histo src  <entree_tmp> <sortie_tmp>\n"
        "    %s histo real <entree_tmp> <sortie_tmp>\n"
        "    %s histo all  <histo_max.csv> <histo_src.csv> <histo_real.csv> <histo_all.csv>\n"
        "\n"
        "  Leaks (Mode Pipeline):\n"
        "    %s leaks <fichier_sortie.dat>\n"
        "    (Les données et la commande 'LEAK;ID' sont lues depuis l'entrée standard STDIN)\n"
        "    (Le résultat est écrit dans <fichier_sortie.dat> après vérification des doublons)\n"
        "\n",
        prog, prog, prog, prog
    );
}

int main(int argc, char **argv) {
    //verifie au moins le nom du programme
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    //histogramme
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

        //Sinon 
        usage(argv[0]);
        return 2;
    }

   
    //Leaks
    if (strcmp(argv[1], "leaks") == 0) {
        
        if (argc < 3) {
            fprintf(stderr, "Erreur: Chemin du fichier de sortie manquant pour leaks.\n");
            usage(argv[0]);
            return 1;
        }

        traiter_fuites_stdin(argv[2]);
        
        return 0; 
    }

    //Sinon
    usage(argv[0]);
    return 2;
}