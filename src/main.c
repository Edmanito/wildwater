#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leaks.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: Wildwater <commande>\n");
        return 1;
    }

    if (strcmp(argv[1], "leaks") == 0) {
        if (argc != 6) {
            fprintf(stderr,
                "Usage: Wildwater leaks <data.csv> <ID_USINE> <out.csv> <history.dat>\n");
            return 1;
        }

        return leaks_compute(
            argv[2],  // CSV
            argv[3],  // ID usine
            argv[4],  // fichier sortie
            argv[5]   // historique
        );
    }

    fprintf(stderr, "Erreur : commande inconnue '%s'\n", argv[1]);
    return 2;
}
