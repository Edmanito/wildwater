#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "histo_src.h"
#include "avl.h"

static char *nettoyer_espaces(char *chaine) {
    if (!chaine) return chaine;
    
    while (*chaine == ' ' || *chaine == '\t' || *chaine == '\r' || *chaine == '\n') chaine++;
    
    size_t longueur = strlen(chaine);
    while (longueur > 0) {
        char caractere = chaine[longueur - 1];
        if (caractere == ' ' || caractere == '\t' || caractere == '\r' || caractere == '\n') {
            chaine[longueur - 1] = '\0';
            longueur--;
        } else break;
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

static int extraire_donnees(char *ligne, char **sortie_id, double *sortie_volume) {
    if (!ligne) return 0;
    if (strncmp(ligne, "identifier;", 11) == 0) return 0;

    char *separateur = strchr(ligne, ';');
    if (!separateur) return 0;

    *separateur = '\0';
    char *id_propre = nettoyer_espaces(ligne);
    char *texte_volume = nettoyer_espaces(separateur + 1);

    if (id_propre[0] == '\0') return 0;
    if (texte_volume[0] == '\0') return 0;
    if (strcmp(texte_volume, "-") == 0) return 0;

    char *fin = NULL;
    double valeur = strtod(texte_volume, &fin);
    if (fin == texte_volume) return 0;

    while (*fin == ' ' || *fin == '\t' || *fin == '\r' || *fin == '\n') fin++;
    if (*fin != '\0') return 0;

    *sortie_id = id_propre;
    *sortie_volume = valeur;
    return 1;
}

int generer_histo_src(const char *chemin_entree, const char *chemin_sortie) {
    FILE *flux_entree = fopen(chemin_entree, "r");
    if (!flux_entree) {
        perror("Erreur ouverture entree");
        return 1;
    }

    noeud_avl_t *racine = NULL;
    char *ligne = NULL;
    size_t capacite = 0;

    while (getline(&ligne, &capacite, flux_entree) != -1) {
        char *texte = nettoyer_espaces(ligne);
        if (texte[0] == '\0') continue;

        char *identifiant = NULL;
        double volume = 0.0;
        
        if (!extraire_donnees(texte, &identifiant, &volume)) continue;

        usine_t *usine = avl_trouver(racine, identifiant);
        if (!usine) {
            usine = creer_usine(identifiant);
            if (!usine) {
                free(ligne);
                fclose(flux_entree);
                avl_liberer(racine);
                return 2;
            }
            racine = avl_inserer(racine, usine);
            if (!racine) {
                free(ligne);
                fclose(flux_entree);
                free(usine->id);
                free(usine);
                return 2;
            }
            usine = avl_trouver(racine, identifiant);
        }
        if (usine) usine->src += volume;
    }

    free(ligne);
    fclose(flux_entree);

    FILE *flux_sortie = fopen(chemin_sortie, "w");
    if (!flux_sortie) {
        perror("Erreur ouverture sortie");
        avl_liberer(racine);
        return 5;
    }

    avl_ecrire(racine, flux_sortie, 1);
    fclose(flux_sortie);

    avl_liberer(racine);
    return 0;
}