#include "../include/arbre.h"
#include "../include/leaks.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static double total_fuites_global = 0.0;

// --- Fonction inspirée de ton code pour gérer l'écriture et les doublons ---
static int ecriture_fichier_verifiee(const char* chemin_fichier, char* id, double volume) {
    FILE* f;
    char id_lu[128];
    double volume_lu;
    int trouve = 0;
    
    // Ouverture en mode ajout+lecture (crée le fichier s'il n'existe pas)
    f = fopen(chemin_fichier, "a+");
    if (f == NULL) {
        perror("Erreur ouverture fichier leaks");
        return 3;   
    }

    // IMPORTANT : On se remet au début pour vérifier tout le fichier
    rewind(f);

    // Lecture ligne par ligne. 
    // Format attendu : ID;VOLUME k.m3;
    // On ignore le reste de la ligne avec %*s si besoin, ou on matche exactement le format d'écriture
    while (fscanf(f, "%127[^;];%lf k.m3;\n", id_lu, &volume_lu) == 2) {
        if (strcmp(id, id_lu) == 0) {
            trouve = 1;
            break;
        }
    }

    if (!trouve) {
        // On se place à la fin pour écrire (normalement a+ le fait, mais fseek sécurise après un rewind/lecture)
        fseek(f, 0, SEEK_END);
        
        // Écriture au format demandé : ID;VOLUME k.m3;
        fprintf(f, "%s;%.3f k.m3;\n", id, volume);
        printf("[C] Ajout de l'usine %s (%.3f k.m3) dans le fichier.\n", id, volume);
        
        fclose(f);
        return 1;  
    }
    else {
        printf("[C] L'usine %s est déjà existante. Pas d'écriture.\n", id);
        fclose(f);
        return 2;
    }
}

// --- Logique de calcul (Inchangée) ---
static void propager_flux(Noeud* noeud, double volume_actuel) {
    if (!noeud || !noeud->liste_enfants) return;

    int nombre_enfants = 0;
    for (Arete* a = noeud->liste_enfants; a; a = a->suivant) {
        nombre_enfants++;
    }

    if (nombre_enfants == 0) return;

    double volume_par_tuyau = volume_actuel / nombre_enfants;

    for (Arete* a = noeud->liste_enfants; a; a = a->suivant) {
        double perte = volume_par_tuyau * (a->pourcentage_fuite / 100.0);
        total_fuites_global += perte;
        propager_flux(a->enfant, volume_par_tuyau - perte);
    }
}

// --- Fonction Principale ---
void traiter_fuites_stdin(const char* fichier_sortie) {
    char ligne[512];
    char arg1[128], arg2[128];
    double valeur;

    // Lecture du pipeline Shell
    while (fgets(ligne, sizeof(ligne), stdin)) {
        if (sscanf(ligne, "SOURCE;%[^;];%lf", arg1, &valeur) == 2) {
            ajouter_volume_source(arg1, valeur);
        }
        else if (sscanf(ligne, "PIPE;%[^;];%[^;];%lf", arg1, arg2, &valeur) == 3) {
            ajouter_arete(arg1, arg2, valeur);
        }
        else if (sscanf(ligne, "LEAK;%[^;\n]", arg1) == 1) {
            Noeud* noeud_depart = obtenir_noeud(arg1);
            
            // Calcul
            total_fuites_global = 0.0;
            propager_flux(noeud_depart, noeud_depart->volume_eau);
            
            // ÉCRITURE AVEC VÉRIFICATION
            // On écrit directement dans le fichier passé en paramètre
            ecriture_fichier_verifiee(fichier_sortie, arg1, total_fuites_global);
        }
    }
    liberer_tout();
}