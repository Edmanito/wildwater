#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arbre.h"
#include "leaks.h"

static double total_fuites = 0.0;

static int ecrire_si_nouveau(const char *chemin, char *id, double volume) {
    FILE *fichier;
    char id_lu[128];
    double vol_lu;
    int trouve = 0;
    
    //Ouverture
    fichier = fopen(chemin, "a+");
    if (fichier == NULL) {
        perror("Erreur ouverture fichier leaks");
        return 3;   
    }

    //on repart au début
    rewind(fichier);

    //Lecture ligne par ligne
    while (fscanf(fichier, "%127[^;];%lf k.m3;\n", id_lu, &vol_lu) == 2) {
        if (strcmp(id, id_lu) == 0) {
            trouve = 1;
            break;
        }
    }

    if (!trouve) {
        fseek(fichier, 0, SEEK_END);
        
        //format : ID   ;   VOLUME k.m3
        fprintf(fichier, "%s;%.3f k.m3;\n", id, volume);
        printf("[C] Ajout de l'usine %s (%.3f k.m3) dans le fichier.\n", id, volume);
        
        fclose(fichier);
        return 1;  
    }
    else {
        printf("[C] L'usine %s est déjà existante. Pas d'écriture.\n", id);
        fclose(fichier);
        return 2;
    }
}

static void calculer_fuites_recursif(Noeud *noeud, double volume_entrant) {
    if (!noeud || !noeud->liste_enfants) return;

    int nb_enfants = 0;
    for (Arete *arete = noeud->liste_enfants; arete; arete = arete->suivant) {
        nb_enfants++;
    }

    if (nb_enfants == 0) return;

    double volume_par_tuyau = volume_entrant / nb_enfants;

    for (Arete *arete = noeud->liste_enfants; arete; arete = arete->suivant) {
        double perte = volume_par_tuyau * (arete->pourcentage_fuite / 100.0);
        total_fuites += perte;
        calculer_fuites_recursif(arete->enfant, volume_par_tuyau - perte);
    }
}

//Fonction  
void traiter_fuites_stdin(const char *chemin_sortie) {
    char tampon[512];
    char id1[128], id2[128];
    double valeur;

    while (fgets(tampon, sizeof(tampon), stdin)) {
        if (sscanf(tampon, "SOURCE;%[^;];%lf", id1, &valeur) == 2) {
            ajouter_volume_source(id1, valeur);
        }
        else if (sscanf(tampon, "PIPE;%[^;];%[^;];%lf", id1, id2, &valeur) == 3) {
            ajouter_arete(id1, id2, valeur);
        }
        else if (sscanf(tampon, "LEAK;%[^;\n]", id1) == 1) {
            Noeud *depart = obtenir_noeud(id1);
            
            //Calcul
            total_fuites = 0.0;
            calculer_fuites_recursif(depart, depart->volume_eau);
            
            //verifie
            ecrire_si_nouveau(chemin_sortie, id1, total_fuites);
        }
    }
    liberer_tout();
}