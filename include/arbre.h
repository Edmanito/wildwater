#ifndef ARBRE_H
#define ARBRE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure pour une arête (un tuyau vers un enfant)
typedef struct Arete {
    struct Noeud* enfant;       // Pointeur vers le nœud enfant
    double pourcentage_fuite;   // % de fuite sur ce tronçon
    struct Arete* suivant;      // Liste chaînée des arêtes
} Arete;

// Structure pour un Nœud (Usine, Stockage, Jonction, etc.)
typedef struct Noeud {
    char id[128];               // Identifiant unique
    double volume_eau;          // Volume entrant cumulé (pour l'usine)
    Arete* liste_enfants;       // Tête de la liste des enfants
    
    // Propriétés pour l'AVL
    struct Noeud *gauche;
    struct Noeud *droite;
    int hauteur;
} Noeud;

// Prototypes des fonctions
Noeud* obtenir_noeud(char* id); // Cherche ou crée un nœud
void ajouter_arete(char* id_parent, char* id_enfant, double fuite);
void ajouter_volume_source(char* id_usine, double volume);
void liberer_tout(void);

#endif