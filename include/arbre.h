#ifndef ARBRE_H
#define ARBRE_H

// Structure représentant un nœud d'arbre général
typedef struct{
    char id[64];               // Identifiant du nœud
    int nbEnfants;             // Nombre d’enfants
    struct Noeud **enfants;    // Tableau dynamique de pointeurs vers les enfants
} Noeud;

// Crée un nœud
Noeud* creerNoeud(const char *id);

// Ajoute un enfant
void ajouterEnfant(Noeud *parent, Noeud *enfant);

#endif
