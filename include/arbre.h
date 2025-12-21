#ifndef ARBRE_H
#define ARBRE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Arete {
    struct Noeud* enfant;      
    double pourcentage_fuite;   
    struct Arete* suivant;      
} Arete;

typedef struct Noeud {
    char id[128];              
    double volume_eau;         
    Arete* liste_enfants;       
    
   
    struct Noeud *gauche;
    struct Noeud *droite;
    int hauteur;
} Noeud;

Noeud* obtenir_noeud(char* id); 
void ajouter_arete(char* id_parent, char* id_enfant, double fuite);
void ajouter_volume_source(char* id_usine, double volume);
void liberer_tout(void);

#endif