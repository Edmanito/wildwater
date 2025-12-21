#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- AJOUTE CES LIGNES ICI --- */
#include "histo_src.h"
#include "histo_real.h"
#include "histo_all.h"
#include "leaks.h"
/* ----------------------------- */

// Fonction pour afficher l'aide si les arguments sont incorrects
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
    // 1. Vérification minimale : il faut au moins le nom du programme et une commande
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    // =========================================================
    // PARTIE HISTOGRAMMES (Arguments classiques)
    // =========================================================
    if (strcmp(argv[1], "histo") == 0) {
        if (argc < 3) {
            usage(argv[0]);
            return 1;
        }

        /* Commande : histo src <in> <out> */
        if (strcmp(argv[2], "src") == 0) {
            if (argc != 5) { usage(argv[0]); return 1; }
            // Appel à la fonction définie dans histo_src.c
            return generer_histo_src(argv[3], argv[4]);
        }

        /* Commande : histo real <in> <out> */
        if (strcmp(argv[2], "real") == 0) {
            if (argc != 5) { usage(argv[0]); return 1; }
            // Appel à la fonction définie dans histo_real.c
            return calculer_histo_real(argv[3], argv[4]);
        }

        /* Commande : histo all <max> <src> <real> <out> */
        if (strcmp(argv[2], "all") == 0) {
            if (argc != 7) { usage(argv[0]); return 1; }
            // Appel à la fonction définie dans histo_all.c
            return generer_histo_all(argv[3], argv[4], argv[5], argv[6]);
        }

        // Si le sous-mode histo est inconnu
        usage(argv[0]);
        return 2;
    }

    // =========================================================
    // PARTIE LEAKS (Mode Pipeline avec fichier de sortie)
    // =========================================================
    if (strcmp(argv[1], "leaks") == 0) {
        // Le programme attend maintenant le chemin du fichier de sortie en argument
        // argv[2] doit contenir le chemin vers leaks.dat pour vérifier les doublons
        
        if (argc < 3) {
            fprintf(stderr, "Erreur: Chemin du fichier de sortie manquant pour leaks.\n");
            usage(argv[0]);
            return 1;
        }

        // On appelle la fonction de traitement (définie dans leaks.c)
        // Elle va lire stdin, calculer, et écrire dans argv[2] si l'ID n'existe pas.
        traiter_fuites_stdin(argv[2]);
        
        return 0; // Succès
    }

    // Si la commande principale est inconnue
    usage(argv[0]);
    return 2;
}