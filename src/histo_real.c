#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "histo_real.h"
#include "avl.h"

static int est_espace(char caractere) {
    return (caractere == ' '  || caractere == '\t' || caractere == '\n' ||
            caractere == '\r' || caractere == '\v' || caractere == '\f');
}

static char *nettoyer_espaces(char *chaine) {
    if (!chaine) return chaine;
    
    while (*chaine && est_espace(*chaine)) chaine++;
    
    size_t longueur = strlen(chaine);

    while (longueur > 0 && est_espace(chaine[longueur - 1])) {
        chaine[longueur - 1] = '\0';
        longueur--;
    }
    return chaine;
}

static usine_t *creer_usine(const char *identifiant) {
    usine_t *usine = malloc(sizeof(*usine));
    if (!usine) return NULL;
    
    usine->id = malloc(strlen(identifiant) + 1);
    if (!usine->id) {
        free(usine);
        return NULL;
    }
    strcpy(usine->id, identifiant);
    usine->max = 0.0;
    usine->src = 0.0;
    usine->real = 0.0;
    return usine;
}

static int convertir_double(const char *texte, double *resultat) {
    if (!texte || texte[0] == '\0') return 0;
    if (strcmp(texte, "-") == 0) return 0;

    char *fin = NULL;
    double valeur = strtod(texte, &fin); 
    
    
    if (fin == texte) return 0;

    while (*fin && est_espace(*fin)) fin++;
    if (*fin != '\0') return 0;

    *resultat = valeur;
    return 1;
}

static int extraire_donnees(char *ligne, char *sortie_id, size_t taille_id,
                            double *sortie_volume, double *sortie_fuite)
{
    if (!ligne) return 0;
    if (strncmp(ligne, "identifier;", 11) == 0) return 0;

    //Recherche
    char *separateur1 = strchr(ligne, ';');
    if (!separateur1) return 0;
    *separateur1 = '\0';

    char *suite = separateur1 + 1;
    char *separateur2 = strchr(suite, ';');
    if (!separateur2) return 0;
    *separateur2 = '\0'; 

    //Nettoyage
    char *id_propre = nettoyer_espaces(ligne);
    char *texte_vol = nettoyer_espaces(suite);
    char *texte_fuite = nettoyer_espaces(separateur2 + 1);

    if (id_propre[0] == '\0') return 0;

    double volume = 0.0, fuite = 0.0;
    if (!convertir_double(texte_vol, &volume)) return 0;
    if (!convertir_double(texte_fuite, &fuite)) return 0;

    //Securité
    if (fuite < 0.0) fuite = 0.0;
    if (fuite > 100.0) fuite = 100.0;

    //copie
    strncpy(sortie_id, id_propre, taille_id - 1);
    sortie_id[taille_id - 1] = '\0';

    *sortie_volume = volume;
    *sortie_fuite = fuite;
    return 1;
}

int calculer_histo_real(const char *chemin_entree, const char *chemin_sortie) {
    FILE *flux = fopen(chemin_entree, "r");
    if (!flux) {
        perror("Erreur ouverture entree");
        return 1;
    }

    noeud_avl_t *racine = NULL;
    char *tampon = NULL;
    size_t taille_tampon = 0;

    //lecture
    while (getline(&tampon, &taille_tampon, flux) != -1) {
        char *ligne_propre = nettoyer_espaces(tampon);
        if (ligne_propre[0] == '\0') continue;

        char id[256];
        double volume = 0.0;
        double fuite = 0.0;

        
        if (!extraire_donnees(ligne_propre, id, sizeof(id), &volume, &fuite)) continue;

        double volume_reel = volume * (1.0 - fuite / 100.0);

        usine_t *usine = avl_trouver(racine, id);
        if (!usine) {
            usine = creer_usine(id);
            if (!usine) {
                free(tampon);
                fclose(flux);
                avl_liberer(racine);
                return 2;
            }
            racine = avl_inserer(racine, usine);
            if (!racine) {
                free(tampon);
                fclose(flux);
                free(usine->id);
                free(usine);
                return 2;
            }
            usine = avl_trouver(racine, id);
        }
        if (usine) usine->real += volume_reel;
    }

    free(tampon);
    fclose(flux);

    FILE *sortie = fopen(chemin_sortie, "w");
    if (!sortie) {
        perror("Erreur ouverture sortie");
        avl_liberer(racine);
        return 4;
    }

   
    avl_ecrire(racine, sortie, 2);
    fclose(sortie);

    avl_liberer(racine);
    return 0;
}