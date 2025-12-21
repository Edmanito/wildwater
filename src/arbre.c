#include "../include/arbre.h"

//racine
static Noeud* racine_avl = NULL;

//fonctions

static int max(int a, int b) { return (a > b) ? a : b; }
static int hauteur(Noeud* n) { return n ? n->hauteur : 0; }

static Noeud* nouveau_noeud(const char* id) {
    Noeud* n = calloc(1, sizeof(Noeud));
    if(!n) {
        fprintf(stderr, "Erreur allocation mémoire\n");
        exit(1);
    }
    strncpy(n->id, id, 127);
    n->id[127] = '\0';
    n->hauteur = 1;
    return n;
}

static Noeud* rotation_droite(Noeud* y) {
    Noeud* x = y->gauche;
    Noeud* T2 = x->droite;

    x->droite = y;
    y->gauche = T2;

    y->hauteur = max(hauteur(y->gauche), hauteur(y->droite)) + 1;
    x->hauteur = max(hauteur(x->gauche), hauteur(x->droite)) + 1;

    return x;
}

static Noeud* rotation_gauche(Noeud* x) {
    Noeud* y = x->droite;
    Noeud* T2 = y->gauche;

    y->gauche = x;
    x->droite = T2;

    x->hauteur = max(hauteur(x->gauche), hauteur(x->droite)) + 1;
    y->hauteur = max(hauteur(y->gauche), hauteur(y->droite)) + 1;

    return y;
}

static int obtenir_equilibre(Noeud* n) {
    if (n == NULL) return 0;
    return hauteur(n->gauche) - hauteur(n->droite);
}

static Noeud* inserer_avl(Noeud* noeud, const char* id, Noeud** resultat) {
  
    if (noeud == NULL) {
        *resultat = nouveau_noeud(id);
        return *resultat;
    }

    int cmp = strcmp(id, noeud->id);
    if (cmp < 0)
        noeud->gauche = inserer_avl(noeud->gauche, id, resultat);
    else if (cmp > 0)
        noeud->droite = inserer_avl(noeud->droite, id, resultat);
    else {
        *resultat = noeud;
        return noeud;
    }

    noeud->hauteur = 1 + max(hauteur(noeud->gauche), hauteur(noeud->droite));

    int equilibre = obtenir_equilibre(noeud);

    if (equilibre > 1 && strcmp(id, noeud->gauche->id) < 0)
        return rotation_droite(noeud);

    if (equilibre < -1 && strcmp(id, noeud->droite->id) > 0)
        return rotation_gauche(noeud);

    if (equilibre > 1 && strcmp(id, noeud->gauche->id) > 0) {
        noeud->gauche = rotation_gauche(noeud->gauche);
        return rotation_droite(noeud);
    }

    if (equilibre < -1 && strcmp(id, noeud->droite->id) < 0) {
        noeud->droite = rotation_droite(noeud->droite);
        return rotation_gauche(noeud);
    }

    return noeud;
}


Noeud* obtenir_noeud(const char* id) {
    Noeud* res = NULL;
    racine_avl = inserer_avl(racine_avl, id, &res);
    return res;
}

void ajouter_arete(const char* id_parent, const char* id_enfant, double fuite) {
    Noeud* parent = obtenir_noeud(id_parent);
    Noeud* enfant = obtenir_noeud(id_enfant);

    Arete* nouvelle_arete = malloc(sizeof(Arete));
    if (!nouvelle_arete) exit(1);

    nouvelle_arete->enfant = enfant;
    nouvelle_arete->pourcentage_fuite = fuite;

    nouvelle_arete->suivant = parent->liste_enfants;
    parent->liste_enfants = nouvelle_arete;
}

void ajouter_volume_source(const char* id_usine, double volume) {
    Noeud* n = obtenir_noeud(id_usine);
    n->volume_eau += volume;
}

//free memoire

static void liberer_arbre(Noeud* n) {
    if (!n) return;

    liberer_arbre(n->gauche);
    liberer_arbre(n->droite);

    Arete* courant = n->liste_enfants;
    while (courant) {
        Arete* temp = courant;
        courant = courant->suivant;
        free(temp);
    }

    free(n);
}

void liberer_tout(void) {
    liberer_arbre(racine_avl);
    racine_avl = NULL;
}
